#include "UIWidgetDocument.h"

using namespace SakuEngine;

//============================================================================
//	UIWidgetDocument classMethods
//============================================================================

UIWidgetRefHandle UIWidgetDocument::ExportHandle(const std::string& assetPath) const {

	// ハンドル作成
	UIWidgetRefHandle handle{};
	handle.assetPath = assetPath;
	return handle;
}

uint32_t UIWidgetDocument::AllocateId() {

	uint32_t id = nextId_;
	// 1つ進める
	++nextId_;
	return id;
}

const UIWidgetNode* UIWidgetDocument::GetRootNode() const {

	// ルートノード取得
	return FindNode(rootId_);
}

UIWidgetNode* UIWidgetDocument::FindNode(uint32_t id) {

	// ノード検索
	auto it = nodes_.find(id);
	return (it == nodes_.end()) ? nullptr : &it->second;
}

const UIWidgetNode* UIWidgetDocument::FindNode(uint32_t id) const {

	// ノード検索
	auto it = nodes_.find(id);
	return (it == nodes_.end()) ? nullptr : &it->second;
}

void UIWidgetDocument::CreateDocument() {

	// ノードクリア
	nodes_.clear();
	nextId_ = 1;

	// ルートノード作成
	rootId_ = AllocateId();
	// デフォルト初期化
	UIWidgetNode root{};
	root.id = rootId_;
	root.typeName = "CanvasPanel";
	root.name = "RootCanvas";
	root.parentId = 0;
	root.visibility = UIVisibility::Visible;
	root.enabled = true;

	// 画面全体に引き延ばす
	root.layout.anchors.min = Vector2::AnyInit(0.0f);
	root.layout.anchors.max = Vector2::AnyInit(1.0f);
	root.layout.offsets = UIMargin{ 0.0f,0.0f,0.0f,0.0f };
	root.layout.alignment = Vector2::AnyInit(0.0f);

	// ルートノード登録
	nodes_[rootId_] = std::move(root);
}

uint32_t UIWidgetDocument::AddNode(const std::string& type, uint32_t parentId, const std::string& nameHint) {

	UIWidgetNode* parent = FindNode(parentId);
	if (!parent) {
		return 0;
	}

	// ノード作成
	const uint32_t id = AllocateId();
	UIWidgetNode node{};
	node.id = id;
	node.typeName = type;
	node.name = MakeUniqueName(nameHint.empty() ? type : nameHint);
	node.parentId = parentId;

	// デフォルト
	node.layout.anchors.min = Vector2::AnyInit(0.0f);
	node.layout.anchors.max = Vector2::AnyInit(0.0f);
	node.layout.alignment = Vector2::AnyInit(0.0f);
	// 適当なサイズ
	node.layout.offsets.left = 10.0f;
	node.layout.offsets.top = 10.0f;
	node.layout.offsets.right = 200.0f;
	node.layout.offsets.bottom = 50.0f;

	// 型別デフォルトプロパティ設定
	if (type == "Image") {

		node.properties["textureName"] = std::string{};
	} else if (type == "TextBlock") {

		node.properties["text"] = std::string("Text");
		node.properties["textBindingKey"] = std::string{};
	} else if (type == "Button") {

		node.properties["clickActionKey"] = std::string{};
		node.properties["style.normal"] = std::string{};
		node.properties["style.hovered"] = std::string{};
		node.properties["style.pressed"] = std::string{};
		node.properties["style.disabled"] = std::string{};
	} else if (type == "Slider") {

		node.properties["value"] = 0.0f;
		node.properties["valueBindingKey"] = std::string{};
	}

	// ノード登録
	nodes_[id] = std::move(node);
	parent->children.push_back(id);
	return id;
}

void UIWidgetDocument::CollectSubtree(uint32_t id, std::vector<uint32_t>& out) const {

	const UIWidgetNode* node = FindNode(id);
	if (!node) {
		return;
	}
	out.push_back(id);
	// 再帰的に子ノードも収集
	for (uint32_t child : node->children) {

		CollectSubtree(child, out);
	}
}

bool UIWidgetDocument::RemoveNode(uint32_t id) {

	// ルートノード、存在しないノードは削除不可
	if (id == 0 || id == rootId_) {
		return false;
	}
	UIWidgetNode* node = FindNode(id);
	if (!node) {
		return false;
	}

	// 親ノードから自分を削除
	UIWidgetNode* parent = FindNode(node->parentId);
	if (parent) {

		auto& child = parent->children;
		child.erase(std::remove(child.begin(), child.end(), id), child.end());
	}
	// サブツリー削除
	RemoveSubtree(id);
	return true;
}

bool UIWidgetDocument::ReparentNode(uint32_t id, uint32_t newParentId, int32_t insertIndex) {

	// ルートノード、存在しないノードは親変更不可
	if (id == 0 || id == rootId_) {
		return false;
	}
	if (newParentId == 0) {
		return false;
	}
	if (id == newParentId) {
		return false;
	}

	UIWidgetNode* node = FindNode(id);
	UIWidgetNode* newParent = FindNode(newParentId);
	if (!node || !newParent) {
		return false;
	}

	// 自分の子へ入れるのは禁止
	std::vector<uint32_t> subtree{};
	CollectSubtree(id, subtree);
	if (std::find(subtree.begin(), subtree.end(), newParentId) != subtree.end()) {
		return false;
	}

	// 古い親から外す
	if (UIWidgetNode* oldParent = FindNode(node->parentId)) {

		auto& child = oldParent->children;
		child.erase(std::remove(child.begin(), child.end(), id), child.end());
	}
	node->parentId = newParentId;

	// 新しい親へ挿入
	auto& dst = newParent->children;
	if (insertIndex < 0 || insertIndex <= static_cast<int32_t>(dst.size())) {

		dst.push_back(id);
	} else {

		dst.insert(dst.begin() + insertIndex, id);
	}
	return true;
}

bool UIWidgetDocument::RenameNode(uint32_t id, const std::string& newName) {

	UIWidgetNode* node = FindNode(id);
	if (!node) {
		return false;
	}
	// 名前変更
	node->name = MakeUniqueName(newName);
	return true;
}

void UIWidgetDocument::RemoveSubtree(uint32_t id) {

	// サブツリー収集
	std::vector<uint32_t> all{};
	CollectSubtree(id, all);
	// ノード削除
	for (uint32_t x : all) {

		nodes_.erase(x);
	}
}

std::string UIWidgetDocument::MakeUniqueName(const std::string& base) const {

	// ベース名が空ならデフォルト名を返す
	if (base.empty()) {
		return "Widget";
	}
	// 被らない名前を探す
	std::string name = base;
	int32_t suffix = 1;
	auto exists = [&](const std::string& name) {

		// 名前が存在するか
		for (auto& node : nodes_) {
			if (node.second.name == name) {
				return true;
			}
		}
		// 存在しなければfalseを返して終了
		return false;
		};
	while (exists(name)) {

		// 名前が被るまでサフィックスを付与して探す
		name = base + "_" + std::to_string(suffix++);
	}
	return name;
}

bool UIWidgetDocument::FromJson([[maybe_unused]] const Json& data) {


	return false;
}

void UIWidgetDocument::ToJson([[maybe_unused]] Json& data) const {


}
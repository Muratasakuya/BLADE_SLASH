#include "UIUserWidget.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Mehtods/UIWidgetFactory.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/UISlatePanelWidget.h>

//============================================================================
//	UIUserWidget classMethods
//============================================================================

namespace {

	// 文字列の64bitハッシュ値を計算
	uint64_t Hash64(std::string_view s) {

		// FNV-1a 64
		uint64_t hash = 1469598103934665603ull;
		for (char c : s) {

			hash ^= static_cast<uint8_t>(c);
			hash *= 1099511628211ull;
		}
		return hash;
	}
}

void UIUserWidget::Init(std::string uiGroupName) {

	uiGroupName_ = uiGroupName;
}

void UIUserWidget::SetRoot(std::unique_ptr<UISlateWidget> root) {

	// ルートを設定してインデックス再構築
	root_ = std::move(root);
	RebuildIndex();
}

void UIUserWidget::RebuildIndex() {

	nameToWidget_.clear();
	idToWidget_.clear();
	if (!root_) { return; }

	std::vector<UISlateWidget*> stack;
	stack.push_back(root_.get());

	while (!stack.empty()) {

		UISlateWidget* widget = stack.back();
		stack.pop_back();
		if (!widget) {
			continue;
		}

		// idが未設定なら名前から生成
		if (widget->GetId() == 0) {
			widget->SetId(AllocateWidgetId(widget->GetName()));
		}
		// generation未設定なら1
		if (widget->GetGeneration() == 0) {
			widget->SetGeneration(1);
		}

		// インデックス登録
		idToWidget_[widget->GetId()] = widget;
		nameToWidget_[widget->GetName()] = widget;

		if (widget->IsPanel()) {

			// 子をスタックに追加
			std::vector<UISlateWidget*> children;
			widget->GetChildren(children);
			for (auto* children : children) {

				stack.push_back(children);
			}
		}
	}
}

std::unique_ptr<UISlateWidget> UIUserWidget::CreateWidgetFromJson(const Json& node) {

	// タイプ名取得
	const std::string type = node.value("type", "");
	if (type.empty()) {
		return nullptr;
	}

	// ウィジェット生成
	auto widget = UIWidgetFactory::GetInstance().Create(type);
	if (!widget) {
		return nullptr;
	}

	// プロパティ復元
	widget->FromJson(node);

	// 子がある場合
	if (node.contains("children") && widget->IsPanel()) {
		if (UISlatePanelWidget* panel = static_cast<UISlatePanelWidget*>(widget.get())) {
			for (const auto& children : node["children"]) {

				// 子ウィジェット生成
				auto child = CreateWidgetFromJson(children);
				if (child) {
					panel->AddChild(std::move(child));
				}
			}
		}
	}
	return widget;
}

uint64_t UIUserWidget::AllocateWidgetId(const std::string& nameHint) {

	// 既存に衝突しないように線形探索で回避
	uint64_t base = Hash64(nameHint);
	uint64_t id = base ? base : 1;
	while (idToWidget_.find(id) != idToWidget_.end()) {
		++id;
	}
	return id;
}

void UIUserWidget::Tick(float deltaTime) {

	if (root_) {

		root_->Tick(*this, deltaTime);
	}
}

void UIUserWidget::OnAddedToTree() {

	if (root_) {

		root_->OnAddedToTree(*this);
	}
}

void UIUserWidget::SynchronizeProperties() {

	if (root_) {

		root_->SynchProperties(*this);
	}
}

UISlateWidget* UIUserWidget::FindWidgetByName(const std::string& name) const {

	// 名前からウィジェット参照を取得
	auto it = nameToWidget_.find(name);
	return (it == nameToWidget_.end()) ? nullptr : it->second;
}

UIWidgetHandle UIUserWidget::FindHandleByName(const std::string& name) const {

	auto it = nameToWidget_.find(name);
	if (it == nameToWidget_.end() || !it->second) {
		// ウィジェットが見つからなかった
		return UIWidgetHandle{};
	}
	// ウィジェットのハンドルを返す
	return UIWidgetHandle{ it->second->GetId(), it->second->GetGeneration() };
}

UISlateWidget* UIUserWidget::Resolve(const UIWidgetHandle& handle) const {

	// ハンドルが有効かチェックしてウィジェット参照を取得
	if (!handle.IsValid()) {
		return nullptr;
	}
	// IDからウィジェット参照を取得
	auto it = idToWidget_.find(handle.id);
	if (it == idToWidget_.end()) {
		return nullptr;
	}
	// 世代が一致するか確認
	if (it->second->GetGeneration() != handle.generation) {
		return nullptr;
	}
	return it->second;
}

bool UIUserWidget::LoadFromJson(const Json& data) {

	if (!data.contains("root")) {
		return false;
	}
	// ルートウィジェットを生成
	auto rootNode = CreateWidgetFromJson(data["root"]);
	if (!rootNode) {
		return false;
	}
	// ルートを設定してインデックス再構築
	root_ = std::move(rootNode);
	RebuildIndex();
	return true;
}

void UIUserWidget::SaveToJson(Json& data) const {

	Json root{};
	if (root_) {
		root_->ToJson(root);
	}
	data["root"] = root;
}
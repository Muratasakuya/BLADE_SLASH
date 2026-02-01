#include "UIAsset.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Sprite/UISpriteComponent.h>
#include <Engine/Editor/UI/Component/Text/UITextComponent.h>
#include <Engine/Editor/UI/Component/Transform/UIParentRectTransform.h>
#include <Engine/Editor/UI/Component/Transform/UISpriteTransformComponent.h>
#include <Engine/Editor/UI/Component/Transform/UITextTransformComponent.h>
#include <Engine/Editor/UI/Component/Material/UISpriteMaterialComponent.h>
#include <Engine/Editor/UI/Component/Material/UITextMaterialComponent.h>
#include <Engine/Editor/UI/Component/Selectable/UISelectableComponent.h>
#include <Engine/Editor/UI/Component/InputNavigation/UIInputNavigationComponent.h>
#include <Engine/Editor/UI/Component/Animation/UIStateAnimationComponent.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UIAsset classMethods
//============================================================================

namespace {

	// ハンドルを一意なキーに変換
	uint64_t MakeHandleKey(const UIElement::Handle& handle) {

		return (static_cast<uint64_t>(handle.generation) << 32) | static_cast<uint64_t>(handle.index);
	}

	// サブツリーを走査してhandle->idを割り当てる
	void BuildSubTreeIdMap(const UIAsset& asset, UIElement::Handle node,
		std::unordered_map<uint64_t, uint32_t>& handleToId, uint32_t& nextId) {

		// 要素が存在しなければ終了
		if (!asset.elements.IsAlive(node)) {
			return;
		}

		// すでに登録されていれば終了
		uint64_t key = MakeHandleKey(node);
		if (handleToId.contains(key)) {
			return;
		}

		// idを割り当て
		handleToId[key] = nextId++;

		const UIElement* element = asset.elements.Get(node);
		if (!element) {
			return;
		}

		// 子要素も再帰的に処理
		for (auto child : element->children) {

			BuildSubTreeIdMap(asset, child, handleToId, nextId);
		}
	}

	// サブツリーを走査してjson化
	void ExportElementRecursive(UIAsset& asset, UIElement::Handle node,
		const std::unordered_map<uint64_t, uint32_t>& handleToId, Json& elementsJson) {

		// 要素が存在しなければ終了
		const UIElement* element = asset.elements.Get(node);
		if (!element) {
			return;
		}

		Json elementData{};
		uint32_t id = handleToId.at(MakeHandleKey(node));

		elementData["id"] = id;
		elementData["name"] = element->name;

		// parent、Prefab内に存在する場合のみID化、無ければ-1
		int32_t parentId = -1;
		if (asset.elements.IsAlive(element->parentHandle)) {

			uint64_t pkey = MakeHandleKey(element->parentHandle);
			auto it = handleToId.find(pkey);
			if (it != handleToId.end()) {

				parentId = static_cast<int32_t>(it->second);
			}
		}
		elementData["parent"] = parentId;

		// children、Prefab内のIDとして保存
		Json childrenJson = Json::array();
		for (auto child : element->children) {

			if (!asset.elements.IsAlive(child)) {
				continue;
			}

			uint64_t childKey = MakeHandleKey(child);
			auto it = handleToId.find(childKey);
			if (it != handleToId.end()) {
				childrenJson.push_back(it->second);
			}
		}
		elementData["children"] = childrenJson;

		// components
		Json compsJson = Json::array();
		for (auto compHandle : element->components) {

			UIComponentSlot* slot = asset.components.Get(compHandle);
			if (!slot || !slot->component) {
				continue;
			}

			IUIComponent* component = slot->component.get();

			Json componentData{};
			componentData["type"] = EnumAdapter<UIComponentType>::ToString(component->GetType());
			component->ToJson(componentData["data"]);
			compsJson.push_back(componentData);
		}
		elementData["components"] = compsJson;

		elementsJson.push_back(elementData);

		// 子要素も再帰的に処理
		for (auto child : element->children) {

			uint64_t childKey = MakeHandleKey(child);
			if (!handleToId.contains(childKey)) {
				continue;
			}
			ExportElementRecursive(asset, child, handleToId, elementsJson);
		}
	}
}

void UIAsset::Init() {

	// クリア
	elements.Clear();
	components.Clear();
	nextUid = 1;
	// ルートのみ作成
	UIElement root{};
	root.name = "Root";
	root.uid = AllocateUid();
	rootHandle = elements.Emplace(root);
}

bool UIAsset::AddChild(UIElement::Handle parent, UIElement::Handle child) {

	UIElement* parentElement = elements.Get(parent);
	UIElement* childElement = elements.Get(child);
	if (parentElement && childElement) {

		// すでに親がいる場合は親から削除
		if (elements.IsAlive(childElement->parentHandle)) {

			RemoveChild(childElement->parentHandle, child);
		}

		// 親子関係を設定
		childElement->parentHandle = parent;
		parentElement->children.emplace_back(child);
		return true;
	}
	return false;
}

bool UIAsset::RemoveChild(UIElement::Handle parent, UIElement::Handle child) {

	UIElement* parentElement = elements.Get(parent);
	if (parentElement) {

		auto& children = parentElement->children;
		// 子要素リストから削除
		// ハンドルが等しいかどうかを比較するラムダ
		children.erase(std::remove_if(children.begin(), children.end(),
			[&](const auto& handle) { return UIElement::Handle::Equal(handle, child); }), children.end());
		return true;
	}
	return false;
}

void UIAsset::DestroyRecursive(UIElement::Handle target) {

	UIElement* element = elements.Get(target);
	if (element) {

		// 子を先に削除
		auto childrenCopy = element->children;
		for (auto child : childrenCopy) {

			DestroyRecursive(child);
		}

		// 親から切り離す
		if (elements.IsAlive(element->parentHandle)) {

			RemoveChild(element->parentHandle, target);
		}

		// 最後に自分を削除
		elements.Destroy(target);
	}
}

bool UIAsset::Reparent(UIElement::Handle child, UIElement::Handle newParent) {

	// ルート要素を親に設定することはできない
	if (UIElement::Handle::Equal(child, rootHandle)) {
		return false;
	}
	// 新しい親に子を追加
	return AddChild(newParent, child);
}

UIComponentHandle UIAsset::AddComponentByType(UIElement::Handle owner, UIComponentType type) {

	switch (type) {
	case UIComponentType::ParentRectTransform:

		return AddComponent<UIParentRectTransform>(owner);
	case UIComponentType::SpriteTransform:

		return AddComponent<UISpriteTransformComponent>(owner);
	case UIComponentType::TextTransform:

		return AddComponent<UITextTransformComponent>(owner);
	case UIComponentType::SpriteMaterial:

		return AddComponent<UISpriteMaterialComponent>(owner);
	case UIComponentType::TextMaterial:

		return AddComponent<UITextMaterialComponent>(owner);
	case UIComponentType::Sprite:

		return AddComponent<UISpriteComponent>(owner);
	case UIComponentType::Text:

		return AddComponent<UITextComponent>(owner);
	case UIComponentType::Selectable:

		return AddComponent<UISelectableComponent>(owner);
	case UIComponentType::InputNavigation:

		return AddComponent<UIInputNavigationComponent>(owner);
	case UIComponentType::StateAnimation:

		return AddComponent<UIStateAnimationComponent>(owner);
	}
	return {};
}

IUIComponent* UIAsset::GetComponent(const UIComponentHandle& handle) {

	UIComponentSlot* slot = components.Get(handle);
	if (!slot || !slot->component) {
		return nullptr;
	}
	return slot->component.get();
}

IUIComponent* UIAsset::FindComponent(UIElement::Handle owner, UIComponentType type) {

	UIElement* element = Get(owner);
	if (!element) {
		return nullptr;
	}

	// 所持しているコンポーネントを走査
	for (const auto& handle : element->components) {

		UIComponentSlot* slot = components.Get(handle);
		if (!slot || !slot->component) {
			continue;
		}

		// タイプが一致したら返す
		if (slot->component->GetType() == type) {
			return slot->component.get();
		}
	}
	return nullptr;
}

void UIAsset::FromJson(const Json& data) {

	if (data.empty()) {
		Init();
		return;
	}

	//============================================================================
	//	初期化
	//============================================================================

	elements.Clear();
	components.Clear();

	//============================================================================
	//	version
	//============================================================================

	uint32_t version = data.value("version", 1u);
	nextUid = data.value("nextUid", 1u);

	//============================================================================
	//	UIElementを作る (d -> handleを構築)
	//============================================================================

	std::unordered_map<uint32_t, UIElement::Handle> idToHandle;

	const auto& elementArray = data["elements"];
	uint32_t maxUid = 0;
	std::vector<UIElement::Handle> needAssign{};
	for (const auto& elementData : elementArray) {

		uint32_t id = elementData["id"].get<uint32_t>();

		UIElement element{};
		element.name = elementData["name"].get<std::string>();

		if (elementData.contains("uid")) {

			element.uid = elementData["uid"].get<uint32_t>();
			maxUid = (std::max)(maxUid, element.uid);
		} else {

			element.uid = 0;
		}

		UIElement::Handle handle = elements.Emplace(element);
		idToHandle[id] = handle;

		if (element.uid == 0) {
			needAssign.push_back(handle);
		}
	}

	// rootHandle
	uint32_t rootId = data["rootId"].get<uint32_t>();
	rootHandle = idToHandle[rootId];

	// nextUid補正
	if (version < 2 || !data.contains("nextUid")) {

		nextUid = (std::max)(nextUid, maxUid + 1);
	} else {

		// 安全のためmax+1より小さければ補正
		nextUid = (std::max)(nextUid, maxUid + 1);
	}

	// uid無しの要素に採番
	for (auto h : needAssign) {
		if (auto* element = elements.Get(h)) {

			element->uid = AllocateUid();
		}
	}

	//============================================================================
	//	親子関係復元 (parent -> child)
	//============================================================================

	for (const auto& element : elementArray) {

		uint32_t id = element["id"].get<uint32_t>();
		int32_t parentId = element["parent"].get<int32_t>();

		// 親がいない場合はスキップ
		if (parentId < 0) {
			continue;
		}

		// 親子関係を復元
		UIElement::Handle parentHandle = idToHandle[static_cast<uint32_t>(parentId)];
		UIElement::Handle childHandle = idToHandle[id];
		AddChild(parentHandle, childHandle);
	}

	//============================================================================
	//	コンポーネント復元
	//============================================================================

	for (const auto& element : elementArray) {

		uint32_t id = element["id"].get<uint32_t>();
		UIElement::Handle owner = idToHandle[id];
		if (!element.contains("components")) {
			continue;
		}
		for (const auto& componentData : element["components"]) {

			UIComponentType type = EnumAdapter<UIComponentType>::FromString(componentData["type"]).value();
			if (IUIComponent* component = GetComponent(AddComponentByType(owner, type))) {

				component->FromJson(componentData["data"]);
			}
		}
	}
}

void UIAsset::ToJson(Json& data) {

	data["version"] = 2;
	data["nextUid"] = nextUid;

	//============================================================================
	//	Handle -> ElementIdの採番
	//============================================================================

	std::unordered_map<uint64_t, uint32_t> handleToId;
	uint32_t nextId = 0;
	// 各エレメントにIDを割り振る
	elements.ForEachAlive([&](UIElement::Handle handle, [[maybe_unused]] const UIElement& element) {

		handleToId[MakeHandleKey(handle)] = nextId++;
		});
	// rootId
	data["rootId"] = handleToId.at(MakeHandleKey(rootHandle));

	//============================================================================
	//	elements
	//============================================================================

	Json elementsJson = Json::array();
	elements.ForEachAlive([&](UIElement::Handle handle, const UIElement& element) {

		Json elementData{};
		uint32_t id = handleToId.at(MakeHandleKey(handle));

		elementData["id"] = id;
		elementData["uid"] = element.uid;
		elementData["name"] = element.name;

		// 親ハンドル
		int32_t parentId = -1;
		if (elements.IsAlive(element.parentHandle)) {
			parentId = static_cast<int32_t>(handleToId.at(MakeHandleKey(element.parentHandle)));
		}
		elementData["parent"] = parentId;

		// 子ハンドル
		Json childrenJson = Json::array();
		for (auto child : element.children) {

			if (!elements.IsAlive(child)) {
				continue;
			}
			childrenJson.push_back(handleToId.at(MakeHandleKey(child)));
		}
		elementData["children"] = childrenJson;

		// 所有しているコンポーネント
		Json compsJson = Json::array();
		for (auto compHandle : element.components) {

			// コンポーネントスロットを取得
			UIComponentSlot* slot = components.Get(compHandle);
			if (!slot || !slot->component) {
				continue;
			}

			// コンポーネントの実体を取得
			IUIComponent* component = slot->component.get();

			Json componentData{};
			componentData["type"] = EnumAdapter<UIComponentType>::ToString(component->GetType());
			component->ToJson(componentData["data"]);
			compsJson.push_back(componentData);
		}
		elementData["components"] = compsJson;

		// エレメントデータを追加
		elementsJson.push_back(elementData);
		});
	data["elements"] = elementsJson;
}

UIElement::Handle UIAsset::ImportJsonElementPrefab(const Json& data, const UIElement::Handle parent) {

	// 要素が存在しなければ空を返す
	if (data.empty() || !data.contains("elements")) {
		return {};
	}

	UIElement::Handle parentRootHandle = parent;

	// 親が無効ならrootを親にする
	if (!elements.IsAlive(parent)) {

		parentRootHandle = rootHandle;
	}

	const auto& elementArray = data["elements"];

	//============================================================================
	//	要素生成（id -> handle）
	//============================================================================

	std::unordered_map<uint32_t, UIElement::Handle> idToHandle;

	for (const auto& elementData : elementArray) {

		uint32_t id = elementData["id"].get<uint32_t>();

		UIElement element{};
		element.name = elementData["name"].get<std::string>();

		UIElement::Handle handle = elements.Emplace(element);
		idToHandle[id] = handle;
	}

	// PrefabのrootHandle
	uint32_t rootId = data["rootId"].get<uint32_t>();
	UIElement::Handle prefabRoot = idToHandle[rootId];

	//============================================================================
	//	親子復元（Prefab内）
	//============================================================================

	for (const auto& elementData : elementArray) {

		uint32_t id = elementData["id"].get<uint32_t>();
		int32_t parentId = elementData["parent"].get<int32_t>();

		// Prefab内親が無い = Prefabルート候補
		if (parentId < 0) {
			continue;
		}

		UIElement::Handle parentHandle = idToHandle[static_cast<uint32_t>(parentId)];
		UIElement::Handle childHandle = idToHandle[id];
		AddChild(parentHandle, childHandle);
	}

	//============================================================================
	//	Prefabルートを指定parentの子にする
	//============================================================================

	AddChild(parentRootHandle, prefabRoot);

	//============================================================================
	//	コンポーネント復元
	//============================================================================

	for (const auto& elementData : elementArray) {

		uint32_t id = elementData["id"].get<uint32_t>();
		UIElement::Handle owner = idToHandle[id];
		if (!elementData.contains("components")) {
			continue;
		}
		for (const auto& componentData : elementData["components"]) {

			UIComponentType type = EnumAdapter<UIComponentType>::FromString(componentData["type"]).value();
			if (IUIComponent* component = GetComponent(AddComponentByType(owner, type))) {

				component->FromJson(componentData["data"]);
			}
		}
	}
	return prefabRoot;
}

void UIAsset::ExportJsonElementPrefab(Json& data, const UIElement::Handle inputRootHandle) {

	data["version"] = 1;

	// 要素が存在しなければ空を返す
	if (!elements.IsAlive(inputRootHandle)) {
		// 空を返す
		data["rootId"] = 0;
		data["elements"] = Json::array();
	}

	//============================================================================
	//	Handle -> PrefabLocalIdをサブツリーのみ採番
	//============================================================================

	std::unordered_map<uint64_t, uint32_t> handleToId;
	uint32_t nextId = 0;
	BuildSubTreeIdMap(*this, inputRootHandle, handleToId, nextId);

	data["rootId"] = handleToId.at(MakeHandleKey(inputRootHandle));

	//============================================================================
	//	elements、サブツリーのみ書き出し
	//============================================================================

	Json elementsJson = Json::array();
	ExportElementRecursive(*this, inputRootHandle, handleToId, elementsJson);
	data["elements"] = elementsJson;
}
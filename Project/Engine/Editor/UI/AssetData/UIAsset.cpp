#include "UIAsset.h"

using namespace SakuEngine;

//============================================================================
//	UIAsset classMethods
//============================================================================

void UIAsset::Init() {

	// クリアしてルートのみ作成
	elements.Clear();
	rootHandle = elements.Emplace(UIElement{ .name = "Root" });
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
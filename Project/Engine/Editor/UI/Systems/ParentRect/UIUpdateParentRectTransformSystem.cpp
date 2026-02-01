#include "UIUpdateParentRectTransformSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

//============================================================================
//	UIUpdateParentRectTransformSystem classMethods
//============================================================================

void UIUpdateParentRectTransformSystem::Update([[maybe_unused]] UISystemContext* context, UIAsset& asset) {

	// ルートから更新
	UpdateRecursive(asset, asset.rootHandle, nullptr);
}

void UIUpdateParentRectTransformSystem::UpdateRecursive(UIAsset& asset,
	const UIElement::Handle& node, const UIParentRectTransform* parent) {

	UIElement* element = asset.Get(node);
	if (!element) {
		return;
	}

	// このノードのトランスフォームを取得
	auto* rectTransform = static_cast<UIParentRectTransform*>(asset.FindComponent(node, UIComponentType::ParentRectTransform));
	if (rectTransform) {

		// 親子付け
		if (parent) {

			rectTransform->transform.parent = &parent->transform;
		} else {

			rectTransform->transform.parent = nullptr;
		}

		// 行列更新
		rectTransform->transform.UpdateMatrix();
		// このノードが親になる
		parent = rectTransform;
	}

	// 子要素に対して再帰的に処理
	for (const auto& child : element->children) {

		UpdateRecursive(asset, child, parent);
	}
}
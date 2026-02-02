#include "UIAnimationAccess.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/Transform/UIParentRectTransform.h>
#include <Engine/Editor/UI/Components/Transform/UISpriteTransformComponent.h>
#include <Engine/Editor/UI/Components/Transform/UITextTransformComponent.h>
#include <Engine/Editor/UI/Components/Material/UISpriteMaterialComponent.h>
#include <Engine/Editor/UI/Components/Material/UITextMaterialComponent.h>

//============================================================================
//	UIAnimationAccess classMethods
//============================================================================

UIPropertyAccess<Vector2> SakuEngine::BuildTranslationAccess(UIAsset& asset, const UIElement::Handle& handle, CanvasType canvasType) {

	UIPropertyAccess<Vector2> access{};
	if (canvasType == CanvasType::Sprite) {

		auto* transform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(handle, UIComponentType::SpriteTransform));
		// 値の取得
		access.getter = [transform]()->Vector2 { return transform->transform->translation; };
		// 値の設定
		access.setter = [transform](const Vector2& value) { transform->transform->translation = value; };
	} else if (canvasType == CanvasType::Text) {

		auto* transform = static_cast<UITextTransformComponent*>(asset.FindComponent(handle, UIComponentType::TextTransform));
		// 値の取得
		access.getter = [transform]()->Vector2 { return transform->transform->translation; };
		// 値の設定
		access.setter = [transform](const Vector2& value) { transform->transform->translation = value; };
	}
	return access;
}

UIPropertyAccess<float> SakuEngine::BuildRotationAccess(UIAsset& asset, const UIElement::Handle& handle, CanvasType canvasType) {

	UIPropertyAccess<float> access{};
	if (canvasType == CanvasType::Sprite) {

		auto* transform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(handle, UIComponentType::SpriteTransform));
		// 値の取得
		access.getter = [transform]()->float { return transform->transform->rotation; };
		// 値の設定
		access.setter = [transform](const float& value) { transform->transform->rotation = value; };
	} else if (canvasType == CanvasType::Text) {

		auto* transform = static_cast<UITextTransformComponent*>(asset.FindComponent(handle, UIComponentType::TextTransform));
		// 値の取得
		access.getter = [transform]()->float { return transform->transform->rotation; };
		// 値の設定
		access.setter = [transform](const float& value) { transform->transform->rotation = value; };
	}
	return access;
}

UIPropertyAccess<Vector2> SakuEngine::BuildScaleAccess(UIAsset& asset, const UIElement::Handle& handle, CanvasType canvasType) {

	UIPropertyAccess<Vector2> access{};
	if (canvasType == CanvasType::Sprite) {

		auto* transform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(handle, UIComponentType::SpriteTransform));
		// 値の取得
		access.getter = [transform]()->Vector2 { return transform->transform->sizeScale; };
		// 値の設定
		access.setter = [transform](const Vector2& value) { transform->transform->sizeScale = value; };
	} else if (canvasType == CanvasType::Text) {

		auto* transform = static_cast<UITextTransformComponent*>(asset.FindComponent(handle, UIComponentType::TextTransform));
		// 値の取得
		access.getter = [transform]()->Vector2 { return transform->transform->sizeScale; };
		// 値の設定
		access.setter = [transform](const Vector2& value) { transform->transform->sizeScale = value; };
	}
	return access;
}

UIPropertyAccess<Color> SakuEngine::BuildColorAccess(UIAsset& asset, const UIElement::Handle& handle, CanvasType canvasType) {

	UIPropertyAccess<Color> access{};
	if (canvasType == CanvasType::Sprite) {

		auto* material = static_cast<UISpriteMaterialComponent*>(asset.FindComponent(handle, UIComponentType::SpriteMaterial));
		// 値の取得
		access.getter = [material]()->Color { return material->material->material.color; };
		// 値の設定
		access.setter = [material](const Color& value) { material->material->material.color = value; };
	} else if (canvasType == CanvasType::Text) {

		auto* material = static_cast<UITextMaterialComponent*>(asset.FindComponent(handle, UIComponentType::TextMaterial));
		// 値の取得
		access.getter = [material]()->Color { return material->material->material.color; };
		// 値の設定
		access.setter = [material](const Color& value) { material->material->material.color = value; };
	}
	return access;
}
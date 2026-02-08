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

namespace {

	// トランスフォームアクセスの構築
	template <typename T>
	UIPropertyAccess<T> BuildBaseTransformAccess(BaseTransform2D& transform, UIAnimationProperty property) {

		UIPropertyAccess<T> access{};
		BaseTransform2D* t = &transform;

		// プロパティに応じてgetter/setterを設定
		switch (property) {
		case UIAnimationProperty::Translation:
			if constexpr (std::is_same_v<T, Vector2>) {

				access.getter = [t]() -> Vector2 { return t->translation; };
				access.setter = [t](const Vector2& value) { t->translation = value; };
			}
			break;

		case UIAnimationProperty::Scale:
			if constexpr (std::is_same_v<T, Vector2>) {

				access.getter = [t]() -> Vector2 { return t->sizeScale; };
				access.setter = [t](const Vector2& value) { t->sizeScale = value; };
			}
			break;

		case UIAnimationProperty::Rotation:
			if constexpr (std::is_same_v<T, float>) {

				access.getter = [t]() -> float { return static_cast<float>(t->rotation); };
				access.setter = [t](const float& value) { t->rotation = value; };
			}
			break;
		}
		return access;
	}
}

UIPropertyAccess<Vector2> SakuEngine::BuildVec2Access(UIAsset& asset, const UIElement::Handle& handle,
	UIAnimationTargetType target, UIAnimationProperty property) {

	UIPropertyAccess<Vector2> access{};
	switch (target) {
	case UIAnimationTargetType::ParentRectTransform: {

		auto* transform = static_cast<UIParentRectTransform*>(asset.FindComponent(handle, UIComponentType::ParentRectTransform));
		access = BuildBaseTransformAccess<Vector2>(transform->transform, property);
		break;
	}
	case UIAnimationTargetType::SpriteTransform: {

		auto* transform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(handle, UIComponentType::SpriteTransform));
		access = BuildBaseTransformAccess<Vector2>(*transform->transform, property);
		break;
	}
	case UIAnimationTargetType::TextTransform: {

		auto* transform = static_cast<UITextTransformComponent*>(asset.FindComponent(handle, UIComponentType::TextTransform));
		access = BuildBaseTransformAccess<Vector2>(*transform->transform, property);
		break;
	}
	}
	return access;
}

UIPropertyAccess<float> SakuEngine::BuildFloatAccess(UIAsset& asset, const UIElement::Handle& handle,
	UIAnimationTargetType target, UIAnimationProperty property) {

	UIPropertyAccess<float> access{};
	switch (target) {
	case UIAnimationTargetType::ParentRectTransform: {

		auto* transform = static_cast<UIParentRectTransform*>(asset.FindComponent(handle, UIComponentType::ParentRectTransform));
		access = BuildBaseTransformAccess<float>(transform->transform, property);
		break;
	}
	case UIAnimationTargetType::SpriteTransform: {

		auto* transform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(handle, UIComponentType::SpriteTransform));
		access = BuildBaseTransformAccess<float>(*transform->transform, property);
		break;
	}
	case UIAnimationTargetType::TextTransform: {

		auto* transform = static_cast<UITextTransformComponent*>(asset.FindComponent(handle, UIComponentType::TextTransform));
		access = BuildBaseTransformAccess<float>(*transform->transform, property);
		break;
	}
	}
	return access;
}

UIPropertyAccess<Color> SakuEngine::BuildColorAccess(UIAsset& asset, const UIElement::Handle& handle,
	UIAnimationTargetType target, [[maybe_unused]] UIAnimationProperty property) {

	UIPropertyAccess<Color> access{};
	switch (target) {
	case UIAnimationTargetType::SpriteMaterial: {

		auto* material = static_cast<UISpriteMaterialComponent*>(asset.FindComponent(handle, UIComponentType::SpriteMaterial));
		access.getter = [material]()-> Color { return material->material->material.color; };
		access.setter = [material](const Color& value) { material->material->material.color = value; };
		break;
	}
	case UIAnimationTargetType::TextMaterial:

		auto* material = static_cast<UITextMaterialComponent*>(asset.FindComponent(handle, UIComponentType::TextMaterial));
		access.getter = [material]()-> Color { return material->material->material.color; };
		access.setter = [material](const Color& value) { material->material->material.color = value; };
		break;
	}
	return access;
}
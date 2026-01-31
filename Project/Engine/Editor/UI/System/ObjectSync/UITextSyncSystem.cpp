#include "UITextSyncSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/UI/Component/Text/UITextComponent.h>
#include <Engine/Editor/UI/Component/Transform/UITextTransformComponent.h>
#include <Engine/Editor/UI/Component/Transform/UIParentRectTransform.h>
#include <Engine/Editor/UI/Component/Material/UITextMaterialComponent.h>

//============================================================================
//	UITextSyncSystem classMethods
//============================================================================

void UITextSyncSystem::Update(UIAsset& asset) {

	// ルートから再帰的に処理
	UpdateRecursive(asset, asset.rootHandle);
}

void UITextSyncSystem::UpdateRecursive(UIAsset& asset, const UIElement::Handle& node) {

	UIElement* element = asset.Get(node);
	if (!element) {
		return;
	}

	// コンポーネントが追加されていたらスプライトオブジェクトを作成する
	EnsureTextObject(asset, *element, node);

	// 子要素に対して再帰的に処理
	for (const auto& childHandle : element->children) {

		UpdateRecursive(asset, childHandle);
	}
}

void UITextSyncSystem::EnsureTextObject(UIAsset& asset, const UIElement& element, const UIElement::Handle& node) {

	// コンポーネント取得
	auto* text = static_cast<UITextComponent*>(asset.FindComponent(node, UIComponentType::Text));
	auto* transform = static_cast<UITextTransformComponent*>(asset.FindComponent(node, UIComponentType::TextTransform));
	auto* material = static_cast<UITextMaterialComponent*>(asset.FindComponent(node, UIComponentType::TextMaterial));
	if (!text || !transform || !material) {
		return;
	}

	// オブジェクトIDが0なら新規作成
	if (text->objectId != 0) {
		return;
	}

	ObjectManager* objManager = ObjectManager::GetInstance();

	// テキストオブジェクトを作成
	text->objectId = objManager->CreateTextObject(text->defaultAtlasTextureName, text->defaultFontPath, element.name, "UIElement");
	// データ取得
	transform->transform = objManager->GetData<TextTransform2D>(text->objectId);
	material->material = objManager->GetData<MSDFTextMaterial>(text->objectId);
	text->text = objManager->GetData<MSDFText>(text->objectId);

	// 生成直後にjson復元を適用
	UISystemMethod::RestoreFromJsonCache(*transform);
	UISystemMethod::RestoreFromJsonCache(*text);
	UISystemMethod::RestoreFromJsonCache(*material);

	// トランスフォームに親子関係を設定
	if (auto* parentComponent = static_cast<UIParentRectTransform*>(asset.FindComponent(
		element.parentHandle, UIComponentType::ParentRectTransform))) {

		transform->transform->parent = &parentComponent->transform;
	}
}
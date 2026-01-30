#include "UITextSyncSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>

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
	auto* text = static_cast<UITextComponent*>(asset.FindComponent(node, UIComponentType::Text));
	auto* transform = static_cast<UITextTransformComponent*>(asset.FindComponent(node, UIComponentType::TextTransform));
	if (text && transform) {

		// 作成
		EnsureTextObject(asset, *element, *text, *transform);
	}

	// 子要素に対して再帰的に処理
	for (const auto& childHandle : element->children) {

		UpdateRecursive(asset, childHandle);
	}
}

void UITextSyncSystem::EnsureTextObject(UIAsset& asset, const UIElement& element,
	UITextComponent& textComponent, UITextTransformComponent& transformComponent) {

	// オブジェクトIDが0なら新規作成
	if (textComponent.objectId != 0) {
		return;
	}

	ObjectManager* objManager = ObjectManager::GetInstance();

	// テキストオブジェクトを作成
	textComponent.objectId = objManager->CreateTextObject(textComponent.defaultAtlasTextureName,
		textComponent.defaultFontPath, element.name, "UIElement");
	// データ取得
	transformComponent.transform = objManager->GetData<TextTransform2D>(textComponent.objectId);
	textComponent.text = objManager->GetData<MSDFText>(textComponent.objectId);

	// 生成直後にjson復元を適用
	UISystemMethod::RestoreFromJsonCache(transformComponent);
	UISystemMethod::RestoreFromJsonCache(textComponent);

	// トランスフォームに親子関係を設定
	if (auto* parentComponent = static_cast<UIParentRectTransform*>(asset.FindComponent(
		element.parentHandle, UIComponentType::ParentRectTransform))) {

		transformComponent.transform->parent = &parentComponent->transform;
	}
}
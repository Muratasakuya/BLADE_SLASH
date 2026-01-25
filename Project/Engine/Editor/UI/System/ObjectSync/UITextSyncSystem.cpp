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

	// トランスフォームの取得
	auto* transform = static_cast<UITextTransformComponent*>(asset.FindComponent(node, UIComponentType::TextTransform));
	if (!transform) {
		return;
	}

	// コンポーネントが追加されていたらテキストオブジェクトを作成する
	if (auto* text = static_cast<UITextComponent*>(asset.FindComponent(node, UIComponentType::Text))) {

		// 作成
		EnsureTextObject(*element, *text);
		// トランスフォーム適用
		ApplyTransform(text->objectId, *transform);
	}

	// 子要素に対して再帰的に処理
	for (const auto& childHandle : element->children) {

		UpdateRecursive(asset, childHandle);
	}
}

void UITextSyncSystem::ApplyTransform(uint32_t objectId, const UITextTransformComponent& component) {

	// トランスフォームデータを取得
	TextTransform2D* transform = ObjectManager::GetInstance()->GetData<TextTransform2D>(objectId);
	if (!transform) {
		return;
	}

	// パラメータを更新
	// SRT
	transform->translation = component.transform.translation;
	transform->rotation = component.transform.rotation;
	transform->sizeScale = component.transform.sizeScale;
	// アンカーポイント
	transform->anchorPoint = component.transform.anchorPoint;
	// テキストボックス関連
	transform->enableTextBox = component.transform.enableTextBox;
	transform->textBoxSize = component.transform.textBoxSize;
	transform->textBoxPadding = component.transform.textBoxPadding;
	transform->lineSpacing = component.transform.lineSpacing;
	// モード
	transform->wrapMode = component.transform.wrapMode;
	transform->verticalAlign = component.transform.verticalAlign;
}

void UITextSyncSystem::EnsureTextObject(const UIElement& element, UITextComponent& component) {

	// オブジェクトIDが0なら新規作成
	if (component.objectId != 0) {
		return;
	}

	// テキストオブジェクトを作成
	component.objectId = ObjectManager::GetInstance()->CreateTextObject(component.atlasTextureName,
		component.fontPath, element.name, "UIElement");
}
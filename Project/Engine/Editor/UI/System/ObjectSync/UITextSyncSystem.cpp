#include "UITextSyncSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/Data/Text/MSDFText.h>

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
		EnsureTextObject(asset, *element, *text);

		// オブジェクト適用
		if (text->objectId != 0) {

			ApplyText(text->objectId, *text);
			ApplyTransform(text->objectId, *transform);
		}
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

void UITextSyncSystem::ApplyText(uint32_t objectId, const UITextComponent& component) {

	// テキストデータを取得
	MSDFText* text = ObjectManager::GetInstance()->GetData<MSDFText>(objectId);
	if (!text) {
		return;
	}

	// 文字列を設定
	text->SetText(component.text);
	// フォントを設定
	text->SetFont(component.atlasTextureName);
}

void UITextSyncSystem::EnsureTextObject(UIAsset& asset, const UIElement& element, UITextComponent& component) {

	// オブジェクトIDが0なら新規作成
	if (component.objectId != 0) {
		return;
	}

	// テキストオブジェクトを作成
	component.objectId = ObjectManager::GetInstance()->CreateTextObject(component.atlasTextureName, component.fontPath, element.name, "UIElement");
	// トランスフォームに親子関係を設定
	auto* parentComponent = static_cast<UIParentRectTransform*>(asset.FindComponent(element.parentHandle, UIComponentType::ParentRectTransform));
	TextTransform2D* transform = ObjectManager::GetInstance()->GetData<TextTransform2D>(component.objectId);
	transform->parent = &parentComponent->transform;
}
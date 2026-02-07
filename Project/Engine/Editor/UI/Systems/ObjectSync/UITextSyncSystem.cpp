#include "UITextSyncSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/UI/Components/Text/UITextComponent.h>
#include <Engine/Editor/UI/Components/Transform/UITextTransformComponent.h>
#include <Engine/Editor/UI/Components/Transform/UIParentRectTransform.h>
#include <Engine/Editor/UI/Components/Material/UITextMaterialComponent.h>

//============================================================================
//	UITextSyncSystem classMethods
//============================================================================

void UITextSyncSystem::Update(UISystemContext* context, UIAsset& asset) {

	// プレビュー抑制判定
	bool canSuppress = context->preview.enabled &&
		(context->preview.clipUid != 0) &&
		context->preview.suppressOriginalSubtree &&
		!context->preview.isPreviewPass &&
		context->preview.element.IsValid() &&
		asset.elements.IsAlive(context->preview.element);

	// ルートから再帰的に処理
	UpdateRecursive(context, asset, asset.rootHandle, canSuppress, false);
}

void UITextSyncSystem::UpdateRecursive(UISystemContext* context, UIAsset& asset,
	const UIElement::Handle& node, bool canSuppress, bool parentSuppressed) {

	UIElement* element = asset.Get(node);
	if (!element) {
		return;
	}

	// プレビュー抑制判定
	bool isSuppressed = parentSuppressed;
	if (canSuppress && UIElement::Handle::Equal(node, context->preview.element)) {
		isSuppressed = true;
	}

	// コンポーネントが追加されていたらスプライトオブジェクトを作成する
	EnsureTextObject(asset, *element, node, isSuppressed);

	// 子要素に対して再帰的に処理
	for (const auto& childHandle : element->children) {

		UpdateRecursive(context, asset, childHandle, canSuppress, isSuppressed);
	}
}

void UITextSyncSystem::EnsureTextObject(UIAsset& asset, const UIElement& element,
	const UIElement::Handle& node, bool suppressed) {

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

	// 存在するなら破棄し、生成はしない
	if (suppressed) {
		if (text->objectId != 0) {

			objManager->Destroy(text->objectId);
			text->objectId = 0;

			text->text = nullptr;
			transform->transform = nullptr;
			material->material = nullptr;
		}
		return;
	}

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
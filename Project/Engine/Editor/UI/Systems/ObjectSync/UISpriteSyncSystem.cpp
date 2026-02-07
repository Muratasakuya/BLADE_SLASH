#include "UISpriteSyncSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/UI/Components/Sprite/UISpriteComponent.h>
#include <Engine/Editor/UI/Components/Transform/UISpriteTransformComponent.h>
#include <Engine/Editor/UI/Components/Transform/UIParentRectTransform.h>
#include <Engine/Editor/UI/Components/Material/UISpriteMaterialComponent.h>

//============================================================================
//	UISpriteSyncSystem classMethods
//============================================================================

void UISpriteSyncSystem::Update(UISystemContext* context, UIAsset& asset) {

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

void UISpriteSyncSystem::UpdateRecursive(UISystemContext* context, UIAsset& asset,
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
	EnsureSpriteObject(asset, *element, node, isSuppressed);

	// 子要素に対して再帰的に処理
	for (const auto& childHandle : element->children) {

		UpdateRecursive(context, asset, childHandle, canSuppress, isSuppressed);
	}
}

void UISpriteSyncSystem::EnsureSpriteObject(UIAsset& asset, const UIElement& element,
	const UIElement::Handle& node, bool suppressed) {

	// コンポーネント取得
	auto* sprite = static_cast<UISpriteComponent*>(asset.FindComponent(node, UIComponentType::Sprite));
	auto* transform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(node, UIComponentType::SpriteTransform));
	auto* material = static_cast<UISpriteMaterialComponent*>(asset.FindComponent(node, UIComponentType::SpriteMaterial));
	if (!sprite || !transform || !material) {
		return;
	}

	// オブジェクトIDが0なら新規作成
	if (sprite->objectId != 0) {
		return;
	}

	ObjectManager* objManager = ObjectManager::GetInstance();

	// 存在するなら破棄し、生成はしない
	if (suppressed) {
		if (sprite->objectId != 0) {

			objManager->Destroy(sprite->objectId);
			sprite->objectId = 0;

			sprite->sprite = nullptr;
			transform->transform = nullptr;
			material->material = nullptr;
		}
		return;
	}

	// オブジェクトを作成してIDを保存
	sprite->objectId = objManager->CreateObject2D(sprite->defaultTextureName, element.name, "UIElement");
	// データ取得
	transform->transform = objManager->GetData<Transform2D>(sprite->objectId);
	sprite->sprite = objManager->GetData<Sprite>(sprite->objectId);
	material->material = objManager->GetData<SpriteMaterial>(sprite->objectId);

	// 生成直後にjson復元を適用
	UISystemMethod::RestoreFromJsonCache(*transform);
	UISystemMethod::RestoreFromJsonCache(*sprite);
	UISystemMethod::RestoreFromJsonCache(*material);

	// トランスフォームに親子関係を設定
	if (auto* parentComponent = static_cast<UIParentRectTransform*>(asset.FindComponent(
		element.parentHandle, UIComponentType::ParentRectTransform))) {

		transform->transform->parent = &parentComponent->transform;
	}
}
#include "UISpriteSyncSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/UI/Component/Sprite/UISpriteComponent.h>
#include <Engine/Editor/UI/Component/Transform/UISpriteTransformComponent.h>
#include <Engine/Editor/UI/Component/Transform/UIParentRectTransform.h>
#include <Engine/Editor/UI/Component/Material/UISpriteMaterialComponent.h>

//============================================================================
//	UISpriteSyncSystem classMethods
//============================================================================

void UISpriteSyncSystem::Update(UIAsset& asset) {

	// ルートから再帰的に処理
	UpdateRecursive(asset, asset.rootHandle);
}

void UISpriteSyncSystem::UpdateRecursive(UIAsset& asset, const UIElement::Handle& node) {

	UIElement* element = asset.Get(node);
	if (!element) {
		return;
	}

	// コンポーネントが追加されていたらスプライトオブジェクトを作成する
	EnsureSpriteObject(asset, *element, node);

	// 子要素に対して再帰的に処理
	for (const auto& childHandle : element->children) {

		UpdateRecursive(asset, childHandle);
	}
}

void UISpriteSyncSystem::EnsureSpriteObject(UIAsset& asset, const UIElement& element, const UIElement::Handle& node) {

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
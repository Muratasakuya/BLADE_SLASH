#include "UISpriteSyncSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>

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
	auto* sprite = static_cast<UISpriteComponent*>(asset.FindComponent(node, UIComponentType::Sprite));
	auto* transform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(node, UIComponentType::SpriteTransform));
	if (sprite && transform) {

		// 作成
		EnsureSpriteObject(asset, *element, *sprite, *transform);
	}

	// 子要素に対して再帰的に処理
	for (const auto& childHandle : element->children) {

		UpdateRecursive(asset, childHandle);
	}
}

void UISpriteSyncSystem::EnsureSpriteObject(UIAsset& asset, const UIElement& element,
	UISpriteComponent& spriteComponent, UISpriteTransformComponent& transformComponent) {

	// オブジェクトIDが0なら新規作成
	if (spriteComponent.objectId != 0) {
		return;
	}

	ObjectManager* objManager = ObjectManager::GetInstance();

	// オブジェクトを作成してIDを保存
	spriteComponent.objectId = objManager->CreateObject2D(spriteComponent.defaultTextureName, element.name, "UIElement");
	// データ取得
	transformComponent.transform = objManager->GetData<Transform2D>(spriteComponent.objectId);
	spriteComponent.sprite = objManager->GetData<Sprite>(spriteComponent.objectId);

	// 生成直後にjson復元を適用
	UISystemMethod::RestoreFromJsonCache(transformComponent);
	UISystemMethod::RestoreFromJsonCache(spriteComponent);

	// トランスフォームに親子関係を設定
	if (auto* parentComponent = static_cast<UIParentRectTransform*>(asset.FindComponent(
		element.parentHandle, UIComponentType::ParentRectTransform))) {

		transformComponent.transform->parent = &parentComponent->transform;
	}
}
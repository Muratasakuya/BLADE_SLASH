#include "UISpriteSyncSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/Data/Sprite/Sprite.h>

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

		// トランスフォーム適用
		if (sprite->objectId != 0) {

			ApplyTransform(sprite->objectId, *transform);
		}
	}

	// 子要素に対して再帰的に処理
	for (const auto& childHandle : element->children) {

		UpdateRecursive(asset, childHandle);
	}
}

void UISpriteSyncSystem::ApplyTransform(uint32_t objectId, const UISpriteTransformComponent& component) {

	// トランスフォームデータを取得
	Transform2D* transform = ObjectManager::GetInstance()->GetData<Transform2D>(objectId);
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
	// サイズ
	transform->size = component.transform.size;
	transform->textureSize = component.transform.textureSize;
	transform->textureLeftTop = component.transform.textureLeftTop;
}

void UISpriteSyncSystem::EnsureSpriteObject(UIAsset& asset, const UIElement& element,
	UISpriteComponent& spriteComponent, UISpriteTransformComponent& transformComponent) {

	// オブジェクトIDが0なら新規作成
	if (spriteComponent.objectId != 0) {
		return;
	}

	// オブジェクトを作成してIDを保存
	spriteComponent.objectId = ObjectManager::GetInstance()->CreateObject2D(spriteComponent.textureName, element.name, "UIElement");
	// データ取得
	Transform2D* transform = ObjectManager::GetInstance()->GetData<Transform2D>(spriteComponent.objectId);
	Sprite* sprite = ObjectManager::GetInstance()->GetData<Sprite>(spriteComponent.objectId);
	// トランスフォームコンポーネントの見た目をスプライトデータに応じて反映
	sprite->SetMetaDataTextureSize(transformComponent.transform);

	// トランスフォームに親子関係を設定
	auto* parentComponent = static_cast<UIParentRectTransform*>(asset.FindComponent(element.parentHandle, UIComponentType::ParentRectTransform));
	transform->parent = &parentComponent->transform;
}
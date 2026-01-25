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

	// トランスフォームの取得
	auto* transform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(node, UIComponentType::SpriteTransform));
	if (!transform) {
		return;
	}

	// コンポーネントが追加されていたらスプライトオブジェクトを作成する
	if (auto* sprite = static_cast<UISpriteComponent*>(asset.FindComponent(node, UIComponentType::Sprite))) {

		// 作成
		EnsureSpriteObject(*element, *sprite);
		// トランスフォーム適用
		ApplyTransform(sprite->objectId, *transform);
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

void UISpriteSyncSystem::EnsureSpriteObject(const UIElement& element, UISpriteComponent& component) {

	// オブジェクトIDが0なら新規作成
	if (component.objectId != 0) {
		return;
	}

	// オブジェクトを作成してIDを保存
	component.objectId = ObjectManager::GetInstance()->CreateObject2D(component.textureName, element.name, "UIElement");
}
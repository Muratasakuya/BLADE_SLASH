#include "UISlateImage.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/UI/Runtime/UIUserWidget.h>
#include <Engine/Object/Data/Transform/Transform.h>
#include <Engine/Object/Data/Sprite/Sprite.h>

//============================================================================
//	UISlateImage classMethods
//============================================================================

void UISlateImage::OnAddedToTree([[maybe_unused]] UIUserWidget& owner) {


}

void UISlateImage::PaintDrawData(UIUserWidget& owner) {

	// 非表示なら何もしない
	if (GetVisibility() == UIVisibility::Collapsed ||
		GetVisibility() == UIVisibility::Hidden) {
		return;
	}

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!isCreated_) {
		// テクスチャ名が空なら何もしない
		if (textureName_.empty()) {
			return;
		}
		// nameを識別子にオブジェクト作成
		spriteObjectId_ = objectManager->CreateObject2D(textureName_, GetName(), owner.GetUIGroupName());
		isCreated_ = true;
	}

	auto* transform = objectManager->GetData<Transform2D>(spriteObjectId_);
	auto* sprite = objectManager->GetData<Sprite>(spriteObjectId_);
	if (!transform || !sprite) {
		return;
	}

	// 描画情報取得
	UIRect rect = GetCachedRect();
	Vector2 pivot = GetLayout().alignment;
	// ピボットを考慮した位置計算
	Vector2 pivotPos = Vector2(rect.pos.x + rect.size.x * pivot.x, rect.pos.y + rect.size.y * pivot.y);

	// トランスフォーム更新
	transform->translation = pivotPos;
	transform->rotation = 0.0f;
	transform->sizeScale = Vector2::AnyInit(1.0f);
	transform->anchorPoint = pivot;
	transform->size = rect.size;

	// テクスチャ名設定
	sprite->SetTextureName(textureName_);
	if (alphaTextureName_.has_value()) {
		sprite->SetAlphaTextureName(alphaTextureName_.value());
	}
}

void UISlateImage::FromJson(const Json& data) {

	UISlateLeafWidget::FromJson(data);
	textureName_ = data.value("textureName", textureName_);
	alphaTextureName_ = data.value("alphaTextureName", std::string{});
	color_ = Color::FromJson(data.value("color", Json()));
}

void UISlateImage::ToJson(Json& data) {

	UISlateLeafWidget::ToJson(data);
	data["textureName"] = textureName_;
	data["alphaTextureName"] = alphaTextureName_.has_value() ? alphaTextureName_.value() : "";
	data["color"] = color_.ToJson();
}
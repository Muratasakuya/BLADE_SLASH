#include "UISlateTextBlock.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/UI/Runtime/UIUserWidget.h>
#include <Engine/Object/Data/Transform/Transform.h>
#include <Engine/Object/Data/Text/MSDFText.h>

//============================================================================
//	UISlateTextBlock classMethods
//============================================================================

void UISlateTextBlock::OnAddedToTree(UIUserWidget& owner) {


}

void UISlateTextBlock::SynchProperties(UIUserWidget& owner) {

	// テキストバインディングキーが設定されていればデータコンテキストから値を取得して反映
	if (!textBindingKey_.empty()) {

		std::string value = owner.Data().GetValue<std::string>(textBindingKey_);
		if (value != text_) {

			text_ = value;
			textDirty_ = true;
		}
	}
}

void UISlateTextBlock::PaintDrawData(UIUserWidget& owner) {

	// 非表示なら何もしない
	if (GetVisibility() == UIVisibility::Collapsed ||
		GetVisibility() == UIVisibility::Hidden) {
		return;
	}

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!isCreated_) {
		// テクスチャ名かフォント情報パスが空なら何もしない
		if (atlasTextureName_.empty() || fontJsonPath_.empty()) {
			return;
		}
		// nameを識別子にテキストオブジェクト作成
		textObjectId_ = objectManager->CreateTextObject(atlasTextureName_, fontJsonPath_, GetName(), owner.GetUIGroupName());
		isCreated_ = true;
	}

	auto* transform = objectManager->GetData<TextTransform2D>(textObjectId_);
	auto* text = objectManager->GetData<MSDFText>(textObjectId_);
	if (!transform || !text) {
		return;
	}

	// 描画情報取得
	 UIRect rect = GetCachedRect();
	 Vector2 pivot = GetLayout().alignment;
	 Vector2 pivotPos = Vector2(rect.pos.x + rect.size.x * pivot.x, rect.pos.y + rect.size.y * pivot.y);

	 // トランスフォーム更新
	 transform->translation = pivotPos;
	 transform->rotation = 0.0f;
	 transform->sizeScale = Vector2::AnyInit(1.0f);
	 transform->anchorPoint = pivot;

	 // 変更を伝える
	if (textDirty_) {

		text->SetText(text_);
		textDirty_ = false;
	}
}

void UISlateTextBlock::FromJson(const Json& data) {

	UISlateLeafWidget::FromJson(data);
	atlasTextureName_ = data.value("atlasTextureName", atlasTextureName_);
	fontJsonPath_ = data.value("fontJsonPath", fontJsonPath_);
	color_ = Color::FromJson(data.value("color", Json()));
	text_ = data.value("text", text_);
	textBindingKey_ = data.value("textBindingKey", textBindingKey_);

	// 文字列変更フラグセット
	textDirty_ = true;
}

void UISlateTextBlock::ToJson(Json& data) {

	UISlateLeafWidget::ToJson(data);
	data["atlasTextureName"] = atlasTextureName_;
	data["fontJsonPath"] = fontJsonPath_;
	data["color"] = color_.ToJson();
	data["text"] = text_;
	data["textBindingKey"] = textBindingKey_;
}
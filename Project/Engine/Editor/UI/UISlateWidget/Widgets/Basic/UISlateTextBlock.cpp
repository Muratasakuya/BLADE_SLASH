#include "UISlateTextBlock.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Runtime/UIUserWidget.h>

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
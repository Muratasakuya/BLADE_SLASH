#include "UISlateImage.h"

using namespace SakuEngine;

//============================================================================
//	UISlateImage classMethods
//============================================================================

void UISlateImage::OnAddedToTree(UIUserWidget& owner) {


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
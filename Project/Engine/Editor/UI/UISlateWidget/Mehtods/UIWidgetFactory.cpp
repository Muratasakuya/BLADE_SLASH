#include "UIWidgetFactory.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateButton.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateImage.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateSlider.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateCanvasPanel.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateTextBlock.h>

//============================================================================
//	UIWidgetFactory classMethods
//============================================================================

UIWidgetFactory& UIWidgetFactory::GetInstance() {

	static UIWidgetFactory instance;
	return instance;
}

std::unique_ptr<UISlateWidget> UIWidgetFactory::Create(const std::string& typeName) const {

	auto it = registry_.find(typeName);
	if (it == registry_.end()) { 
		return nullptr;
	}
	return it->second();
}

void UIWidgetFactory::Register(std::string typeName, CreateFn fn) {

	registry_[std::move(typeName)] = std::move(fn);
}

void UIWidgetFactory::RegisterDefaults() {

	Register("CanvasPanel", []() { return std::make_unique<UISlateCanvasPanel>(); });
	Register("Button", []() { return std::make_unique<UISlateButton>(); });
	Register("Slider", []() { return std::make_unique<UISlateSlider>(); });
	Register("Image", []() { return std::make_unique<UISlateImage>(); });
	Register("TextBlock", []() { return std::make_unique<UISlateTextBlock>(); });
}
#include "UIWidgetFactory.h"

using namespace SakuEngine;

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
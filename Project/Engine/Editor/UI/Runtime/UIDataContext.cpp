#include "UIDataContext.h"

using namespace SakuEngine;

//============================================================================
//	UIDataContext classMethods
//============================================================================

void UIDataContext::SetBool(const std::string& key, bool value) {

	Value valueStruct{};
	valueStruct.type = AnyMoldAll::Bool;
	valueStruct.boolValue = value;
	map_[key] = std::move(valueStruct);
}

void UIDataContext::SetInt(const std::string& key, int32_t value) {

	Value valueStruct{};
	valueStruct.type = AnyMoldAll::Int;
	valueStruct.intValue = value;
	map_[key] = std::move(valueStruct);
}

void UIDataContext::SetFloat(const std::string& key, float value) {

	Value valueStruct{};
	valueStruct.type = AnyMoldAll::Float;
	valueStruct.floatValue = value;
	map_[key] = std::move(valueStruct);
}
void UIDataContext::SetString(const std::string& key, std::string value) {

	Value valueStruct{};
	valueStruct.type = AnyMoldAll::String;
	valueStruct.stringValue = std::move(value);
	map_[key] = std::move(valueStruct);
}

bool UIDataContext::GetBool(const std::string& key) const {

	auto it = map_.find(key);
	if (it != map_.end() && it->second.type == AnyMoldAll::Bool) {
		return it->second.boolValue;
	}
	return false;
}

int32_t UIDataContext::GetInt(const std::string& key) const {

	auto it = map_.find(key);
	if (it != map_.end() && it->second.type == AnyMoldAll::Int) {
		return it->second.intValue;
	}
	return 0;
}

float UIDataContext::GetFloat(const std::string& key) const {

	auto it = map_.find(key);
	if (it != map_.end() && it->second.type == AnyMoldAll::Float) {
		return it->second.floatValue;
	}
	return 0.0f;
}

std::string UIDataContext::GetString(const std::string& key) const {

	auto it = map_.find(key);
	if (it != map_.end() && it->second.type == AnyMoldAll::String) {
		return it->second.stringValue;
	}
	return "";
}
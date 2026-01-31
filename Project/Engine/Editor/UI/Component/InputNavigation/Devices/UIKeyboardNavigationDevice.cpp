#include "UIKeyboardNavigationDevice.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	UIKeyboardNavigationDevice classMethods
//============================================================================

UIKeyboardNavigationDevice::UIKeyboardNavigationDevice(UIInputNavigationComponent* component) :
	component_(component) {

	input_ = Input::GetInstance();
}

float UIKeyboardNavigationDevice::GetVector(UINavigationAction axis) const {

	// いずれかのボタンが押されているか
	auto Any = [&](const std::vector<uint8_t>& keys) {
		for (auto key : keys) {
			if (input_->PushKey(key)) {
				return true;
			}
		}
		return false;
		};

	if (axis == UINavigationAction::MoveX) {

		float v = 0.0f;
		if (Any(component_->rightKeys)) {
			v += 1.0f;
		}
		if (Any(component_->leftKeys)) {
			v -= 1.0f;
		}
		return std::clamp(v, -1.0f, 1.0f);
	}
	if (axis == UINavigationAction::MoveY) {

		float v = 0.0f;
		if (Any(component_->upKeys)) {
			v += 1.0f;
		}
		if (Any(component_->downKeys)) {
			v -= 1.0f;
		}
		return std::clamp(v, -1.0f, 1.0f);
	}
	return 0.0f;
}

bool UIKeyboardNavigationDevice::IsPressed(UINavigationAction button) const {

	// いずれかのボタンが押されているか
	auto Any = [&](const std::vector<uint8_t>& keys) {
		for (auto key : keys) {
			if (input_->PushKey(key)) {
				return true;
			}
		}
		return false;
		};
	if (button == UINavigationAction::Submit) {
		return Any(component_->submitKeys);
	}
	if (button == UINavigationAction::Cancel) {
		return Any(component_->cancelKeys);
	}
	return false;
}

bool UIKeyboardNavigationDevice::IsTriggered(UINavigationAction button) const {

	// いずれかのボタンが押されているか
	auto Any = [&](const std::vector<uint8_t>& keys) {
		for (auto key : keys) {
			if (input_->TriggerKey(key)) {
				return true;
			}
		}
		return false;
		};
	if (button == UINavigationAction::Submit) {
		return Any(component_->submitKeys);
	}
	if (button == UINavigationAction::Cancel) {
		return Any(component_->cancelKeys);
	}
	return false;
}
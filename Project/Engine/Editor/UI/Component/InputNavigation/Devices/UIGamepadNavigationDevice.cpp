#include "UIGamepadNavigationDevice.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	UIGamepadNavigationDevice classMethods
//============================================================================

UIGamepadNavigationDevice::UIGamepadNavigationDevice(UIInputNavigationComponent* component) :
	component_(component) {

	input_ = Input::GetInstance();
}

float UIGamepadNavigationDevice::GetVector(UINavigationAction axis) const {

	// いずれかのボタンが押されているか
	auto Any = [&](const std::vector<GamePadButtons>& buttons) {
		for (auto button : buttons) {
			if (input_->PushGamepadButton(button)) {
				return true;
			}
		}
		return false;
		};

	// スティック値を取得
	const Vector2 leftStickValue = input_->GetLeftStickVal();
	const float maxValue = input_->GetMaxStickValue();
	const float sx = (maxValue != 0.0f) ? (leftStickValue.x / maxValue) : 0.0f;
	const float sy = (maxValue != 0.0f) ? (leftStickValue.y / maxValue) : 0.0f;

	if (axis == UINavigationAction::MoveX) {

		float value = sx;
		if (Any(component_->rightPad)) {
			value += 1.0f;
		}
		if (Any(component_->leftPad)) {
			value -= 1.0f;
		}
		return std::clamp(value, -1.0f, 1.0f);
	}
	if (axis == UINavigationAction::MoveY) {

		float value = sy;
		if (Any(component_->upPad)) {
			value += 1.0f;
		}
		if (Any(component_->downPad)) {
			value -= 1.0f;
		}
		return std::clamp(value, -1.0f, 1.0f);
	}
	return 0.0f;
}

bool UIGamepadNavigationDevice::IsPressed(UINavigationAction button) const {

	// いずれかのボタンが押されているか
	auto Any = [&](const std::vector<GamePadButtons>& buttons) {
		for (auto button : buttons) {
			if (input_->PushGamepadButton(button)) {
				return true;
			}
		}
		return false;
		};
	if (button == UINavigationAction::Submit) {
		return Any(component_->submitPad);
	}
	if (button == UINavigationAction::Cancel) {
		return Any(component_->cancelPad);
	}
	return false;
}

bool UIGamepadNavigationDevice::IsTriggered(UINavigationAction button) const {

	// いずれかのボタンが押されているか
	auto Any = [&](const std::vector<GamePadButtons>& buttons) {
		for (auto button : buttons) {
			if (input_->TriggerGamepadButton(button)) {
				return true;
			}
		}
		return false;
		};
	if (button == UINavigationAction::Submit) {
		return Any(component_->submitPad);
	}
	if (button == UINavigationAction::Cancel) {
		return Any(component_->cancelPad);
	}
	return false;
}
#include "PlayerGamePadInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	PlayerGamePadInput classMethods
//============================================================================

float PlayerGamePadInput::GetVector(PlayerInputAction axis) const {

	// 左スティック入力
	const SakuEngine::Vector2 value = input_->GetLeftStickVal();
	switch (axis) {
	case PlayerInputAction::MoveX: {

		return value.x;
	}
	case PlayerInputAction::MoveZ: {

		return value.y;
	}
	}
	return 0.0f;
}

bool PlayerGamePadInput::IsPressed(PlayerInputAction button) const {

	switch (button) {
	case PlayerInputAction::Dash: {

		return input_->PushGamepadButton(GamePadButtons::A);
	}
	case PlayerInputAction::Avoid: {

		return input_->PushGamepadButton(GamePadButtons::A);
	}
	case PlayerInputAction::Attack: {

		return input_->PushGamepadButton(GamePadButtons::X);
	}
	case PlayerInputAction::Skill: {

		return input_->PushGamepadButton(GamePadButtons::Y);
	}
	case PlayerInputAction::Switching: {

		return input_->PushGamepadButton(GamePadButtons::LEFT_SHOULDER) ||
			input_->PushGamepadButton(GamePadButtons::RIGHT_SHOULDER);
	}
	case PlayerInputAction::NotSwitching: {

		return input_->PushGamepadButton(GamePadButtons::A);
	}
	case PlayerInputAction::Parry: {

		return input_->PushGamepadButton(GamePadButtons::LEFT_SHOULDER) ||
			input_->PushGamepadButton(GamePadButtons::RIGHT_SHOULDER) ||
			input_->PushGamepadButton(GamePadButtons::LEFT_TRIGGER) ||
			input_->PushGamepadButton(GamePadButtons::RIGHT_TRIGGER);
	}
	}
	return false;
}

bool PlayerGamePadInput::IsTriggered(PlayerInputAction button) const {

	switch (button) {
	case PlayerInputAction::Dash: {

		return input_->TriggerGamepadButton(GamePadButtons::A);
	}
	case PlayerInputAction::Avoid: {

		return input_->TriggerGamepadButton(GamePadButtons::A);
	}
	case PlayerInputAction::Attack: {

		return input_->TriggerGamepadButton(GamePadButtons::X);
	}
	case PlayerInputAction::Skill: {

		return input_->TriggerGamepadButton(GamePadButtons::Y);
	}
	case PlayerInputAction::Switching: {

		return input_->TriggerGamepadButton(GamePadButtons::LEFT_SHOULDER) ||
			input_->TriggerGamepadButton(GamePadButtons::RIGHT_SHOULDER);
	}
	case PlayerInputAction::NotSwitching: {

		return input_->TriggerGamepadButton(GamePadButtons::A);
	}
	case PlayerInputAction::Parry: {

		return input_->TriggerGamepadButton(GamePadButtons::LEFT_SHOULDER) ||
			input_->TriggerGamepadButton(GamePadButtons::RIGHT_SHOULDER) ||
			input_->TriggerGamepadButton(GamePadButtons::LEFT_TRIGGER) ||
			input_->TriggerGamepadButton(GamePadButtons::RIGHT_TRIGGER);
	}
	}
	return false;
}
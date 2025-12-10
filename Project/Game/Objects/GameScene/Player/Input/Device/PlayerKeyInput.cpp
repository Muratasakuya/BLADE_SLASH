#include "PlayerKeyInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	PlayerKeyInput classMethods
//============================================================================

float PlayerKeyInput::GetVector(PlayerInputAction axis) const {

	float vector = 0.0f;
	if (axis == PlayerInputAction::MoveX) {
		if (input_->PushKey(DIK_A)) {

			vector -= 1.0f;
		} else if (input_->PushKey(DIK_D)) {

			vector += 1.0f;
		}
		return vector;
	}
	if (axis == PlayerInputAction::MoveZ) {

		if (input_->PushKey(DIK_S)) {

			vector -= 1.0f;
		} else if (input_->PushKey(DIK_W)) {

			vector += 1.0f;
		}
		return vector;
	}
	return vector;
}

bool PlayerKeyInput::IsPressed(PlayerInputAction button) const {

	switch (button) {
	case PlayerInputAction::Dash: {

		return input_->PushMouseRight();
	}
	case PlayerInputAction::Avoid: {

		return input_->PushMouseRight();
	}
	case PlayerInputAction::Attack: {

		return input_->PushMouseLeft();
	}
	case PlayerInputAction::Skill: {

		return input_->PushKey(DIK_E);
	}
	case PlayerInputAction::Switching: {

		return input_->PushMouseLeft() || input_->PushMouseRight();
	}
	case PlayerInputAction::NotSwitching: {

		return input_->PushMouseCenter();
	}
	case PlayerInputAction::Parry: {

		return input_->PushKey(DIK_SPACE);
	}
	}
	return false;
}

bool PlayerKeyInput::IsTriggered(PlayerInputAction button) const {

	switch (button) {
	case PlayerInputAction::Dash: {

		return input_->TriggerMouseRight();
	}
	case PlayerInputAction::Avoid: {

		return input_->TriggerMouseRight();
	}
	case PlayerInputAction::Attack: {

		return input_->TriggerMouseLeft();
	}
	case PlayerInputAction::Skill: {

		return input_->TriggerKey(DIK_E);
	}
	case PlayerInputAction::Switching: {

		return input_->TriggerMouseLeft() || input_->TriggerMouseRight();
	}
	case PlayerInputAction::NotSwitching: {

		return input_->TriggerMouseCenter();
	}
	case PlayerInputAction::Parry: {

		return input_->TriggerKey(DIK_SPACE);
	}
	}
	return false;
}
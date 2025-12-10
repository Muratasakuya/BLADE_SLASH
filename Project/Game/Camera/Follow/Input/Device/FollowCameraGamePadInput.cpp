#include "FollowCameraGamePadInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	FollowCameraGamePadInput classMethods
//============================================================================

float FollowCameraGamePadInput::GetVector(FollowCameraInputAction axis) const {

	const Vector2 value = Input::GetInstance()->GetRightStickVal() * 1.0f / 32767.0f;
	switch (axis) {
	case FollowCameraInputAction::RotateX: {

		return value.x;
	}
	case FollowCameraInputAction::RotateY: {

		return value.y;
	}
	}
	return 0.0f;
}

bool FollowCameraGamePadInput::IsPressed(FollowCameraInputAction button) const {

	switch (button) {
	case FollowCameraInputAction::RotateX: {
		return false;
	}
	case FollowCameraInputAction::RotateY: {
		return false;
	}
	}
	return false;
}

bool FollowCameraGamePadInput::IsTriggered(FollowCameraInputAction button) const {

	switch (button) {
	case FollowCameraInputAction::RotateX: {
		return false;
	}
	case FollowCameraInputAction::RotateY: {
		return false;
	}
	}
	return false;
}
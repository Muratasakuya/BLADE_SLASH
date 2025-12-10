#include "FollowCameraKeyInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	FollowCameraKeyInput classMethods
//============================================================================

float FollowCameraKeyInput::GetVector(FollowCameraInputAction axis) const {

	const Vector2 value = input_->GetMouseMoveValue();
	if (axis == FollowCameraInputAction::RotateX) {

		return value.x;
	}
	if (axis == FollowCameraInputAction::RotateY) {

		return value.y;
	}
	return 0.0f;
}

bool FollowCameraKeyInput::IsPressed(FollowCameraInputAction button) const {

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

bool FollowCameraKeyInput::IsTriggered(FollowCameraInputAction button) const {

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
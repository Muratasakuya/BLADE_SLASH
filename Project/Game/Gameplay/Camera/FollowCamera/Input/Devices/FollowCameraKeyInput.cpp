#include "FollowCameraKeyInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	FollowCameraKeyInput classMethods
//============================================================================

float FollowCameraKeyInput::GetVector(FollowCameraInputAction axis) const {

	SakuEngine::Vector2 value = input_->GetMouseMoveValue();
	if (axis == FollowCameraInputAction::RotateX) {

		return value.x;
	}
	if (axis == FollowCameraInputAction::RotateY) {

		return value.y;
	}
	return 0.0f;
}
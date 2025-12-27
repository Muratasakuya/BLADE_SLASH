#include "FollowCameraGamePadInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Config.h>

//============================================================================
//	FollowCameraGamePadInput classMethods
//============================================================================

float FollowCameraGamePadInput::GetVector(FollowCameraInputAction axis) const {

	// 正規化して返す
	SakuEngine::Vector2 value = input_->GetRightStickVal() * 1.0f / input_->GetMaxStickValue();
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
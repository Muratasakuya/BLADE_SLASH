#include "FollowCameraIState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

//============================================================================
//	FollowCameraIState classMethods
//============================================================================

void FollowCameraIState::SetTarget(FollowCameraTargetType type,
	const SakuEngine::Transform3D& target) {

	targets_[type] = &target;
}
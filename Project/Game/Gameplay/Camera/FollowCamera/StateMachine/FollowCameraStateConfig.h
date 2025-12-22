#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/StateMachineConfig.h>
#include <Game/Gameplay/Camera/FollowCamera/StateMachine/Interface/FollowCameraIState.h>

//============================================================================
//	FollowCameraStateConfig structure
//============================================================================

// カメラ状態機械の設定
struct FollowCameraStateConfig :
	SakuEngine::StateMachineConfig<FollowCameraState, FollowCameraIState,
	SakuEngine::DenseStateStorage<FollowCameraState, FollowCameraIState>> {
};
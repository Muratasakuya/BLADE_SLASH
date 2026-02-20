#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/StateMachineConfig.h>
#include <Game/Gameplay/Actors/Player/Structure/PlayerStructures.h>
#include <Game/Gameplay/Actors/Player/StateMachine/Interface/PlayerIState.h>

//============================================================================
//	PlayerStateConfig structure
//============================================================================

// プレイヤー状態機械の設定
struct PlayerStateConfig :
	SakuEngine::StateMachineConfig<PlayerState, PlayerIState,
	SakuEngine::DenseStateStorage<PlayerState, PlayerIState>> {
};
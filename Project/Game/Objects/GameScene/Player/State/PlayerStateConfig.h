#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/StateMachineConfig.h>
#include <Game/Objects/GameScene/Player/Structure/PlayerStructures.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>

//============================================================================
//	PlayerStateConfig structure
//============================================================================

// プレイヤー状態機械の設定
struct PlayerStateConfig :
	SakuEngine::StateMachineConfig<PlayerState, PlayerIState,
	SakuEngine::DenseStateStorage<PlayerState, PlayerIState>> {
};
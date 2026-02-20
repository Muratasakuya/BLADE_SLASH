#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/StateMachineConfig.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Structures/BossEnemyStructures.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyStateConfig structure
//============================================================================

// プレイヤー状態機械の設定
struct BossEnemyStateConfig :
	SakuEngine::StateMachineConfig<BossEnemyState, BossEnemyIState,
	SakuEngine::DenseStateStorage<BossEnemyState, BossEnemyIState>> {
};
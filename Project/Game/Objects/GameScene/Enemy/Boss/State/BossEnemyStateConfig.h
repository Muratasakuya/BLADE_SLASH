#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/StateMachineConfig.h>
#include <Game/Objects/GameScene/Enemy/Boss/Structures/BossEnemyStructures.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyStateConfig structure
//============================================================================

// プレイヤー状態機械の設定
struct BossEnemyStateConfig :
	SakuEngine::StateMachineConfig<BossEnemyState, BossEnemyIState,
	SakuEngine::DenseStateStorage<BossEnemyState, BossEnemyIState>> {
};
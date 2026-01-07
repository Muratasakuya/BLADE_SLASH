#include "BossEnemyBaseAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyBaseAttackState classMethods
//============================================================================

void BossEnemyBaseAttackState::CheckAndApplySpeedRate() {

	if (isAppliedParrySpeedRate_) {
		return;
	}

	// 待機状態かチェック
	if (player_->GetCurrentState() == PlayerState::ParryWait) {

		// 倍速を適用
		bossEnemy_->SetPlaybackSpeed(attackSpeedRate_);
	}
}

void BossEnemyBaseAttackState::ResetSpeedRate() {

	// 倍速をリセット
	isAppliedParrySpeedRate_ = false;
	bossEnemy_->SetPlaybackSpeed(1.0f);
}
#include "PlayerIState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	PlayerIState classMethods
//============================================================================

PlayerIState::PlayerIState() {

	postProcess_ = nullptr;
	postProcess_ = SakuEngine::PostProcessSystem::GetInstance();
}

void PlayerIState::SetRotateToDirection(const SakuEngine::Vector3& move) {

	SakuEngine::Vector3 direction = SakuEngine::Vector3(move.x, 0.0f, move.z).Normalize();

	if (direction.Length() <= epsilon_) {
		return;
	}

	// 向きを計算
	SakuEngine::Quaternion targetRotation = SakuEngine::Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up));
	SakuEngine::Quaternion rotation = player_->GetRotation();
	rotation = SakuEngine::Quaternion::Slerp(rotation, targetRotation, rotationLerpRate_);
	player_->SetRotation(rotation);
}

SakuEngine::Vector3 PlayerIState::GetPlayerFixedYPos() const {

	SakuEngine::Vector3 translation = player_->GetTranslation();
	// Y座標を固定
	translation.y = 0.0f;

	return translation;
}

SakuEngine::Vector3 PlayerIState::GetBossEnemyFixedYPos() const {

	SakuEngine::Vector3 translation = bossEnemy_->GetTranslation();
	// Y座標を固定
	translation.y = 0.0f;

	return translation;
}

float PlayerIState::GetDistanceToBossEnemy() const {

	// プレイヤーとボス敵の距離を取得
	float distance = SakuEngine::Vector3(GetBossEnemyFixedYPos() - GetPlayerFixedYPos()).Length();
	return distance;
}

SakuEngine::Vector3 PlayerIState::GetDirectionToBossEnemy() const {

	// プレイヤーからボス敵への方向を取得
	SakuEngine::Vector3 direction = SakuEngine::Vector3(GetBossEnemyFixedYPos() - GetPlayerFixedYPos()).Normalize();
	return direction;
}
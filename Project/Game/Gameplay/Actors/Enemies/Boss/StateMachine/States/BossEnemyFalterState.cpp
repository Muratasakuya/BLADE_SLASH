#include "BossEnemyFalterState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyFalterState classMethods
//============================================================================

void BossEnemyFalterState::Enter() {

	bossEnemy_->SetNextAnimation("bossEnemy_falter", false, nextAnimDuration_);

	// Y座標をリセットする
	bossEnemy_->SetTranslation(SakuEngine::Vector3(bossEnemy_->GetTranslation().x, 0.0f, bossEnemy_->GetTranslation().z));
}

void BossEnemyFalterState::Update() {

	const float deltaTime = SakuEngine::GameTimer::GetScaledDeltaTime();

	// 前方ベクトルを取得
	SakuEngine::Vector3 bossPos = bossEnemy_->GetTranslation();
	SakuEngine::Vector3 playerPos = player_->GetTransform().translation;

	// 回転を計算して設定
	SakuEngine::Quaternion bossRotation = SakuEngine::Quaternion::LookTarget(bossPos, playerPos,
		Direction::Get(Direction3D::Up), bossEnemy_->GetRotation(), rotationLerpRate_ * deltaTime);
	bossEnemy_->SetRotation(bossRotation);

	// 後ずさりさせる
	SakuEngine::Vector3 backStepVelocity = bossEnemy_->GetTransform().GetBack() * backStepSpeed_ * deltaTime;
	bossEnemy_->SetTranslation(bossPos + backStepVelocity);

	// アニメーションが終了次第状態を終了
	if (bossEnemy_->IsAnimationFinished()) {

		canExit_ = true;
	}
}

void BossEnemyFalterState::Exit() {
}

void BossEnemyFalterState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("backStepSpeed", &backStepSpeed_, 0.001f);
}

void BossEnemyFalterState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	backStepSpeed_ = SakuEngine::JsonAdapter::GetValue<float>(data, "backStepSpeed_");
}

void BossEnemyFalterState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["backStepSpeed_"] = backStepSpeed_;
}
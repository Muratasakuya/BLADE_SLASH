#include "BossEnemyIdleState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyIdleState classMethods
//============================================================================

void BossEnemyIdleState::Enter() {

	canExit_ = false;
	// animationをリセットする
	bossEnemy_->ResetAnimation();

	bossEnemy_->SetNextAnimation("bossEnemy_idle", true, nextAnimDuration_);
}

void BossEnemyIdleState::Update() {

	// プレイヤーの方を向くようにしておく
	SakuEngine::Math::LookTarget3D(*bossEnemy_, SakuEngine::Math::GetFlattenPos3D(*player_, false), rotationLerpRate_);

	// 後ずさりさせる
	SakuEngine::Vector3 bossPos = bossEnemy_->GetTranslation();
	SakuEngine::Vector3 backStepVelocity = bossEnemy_->GetTransform().GetBack() * backStepSpeed_ * SakuEngine::GameTimer::GetScaledDeltaTime();
	bossEnemy_->SetTranslation(bossPos + backStepVelocity);

	// animationが終了したら遷移可能にする
	if (bossEnemy_->GetAnimationRepeatCount() != 0) {

		canExit_ = true;
	}
}

void BossEnemyIdleState::Exit() {

	canExit_ = false;
	// animationをリセットする
	bossEnemy_->ResetAnimation();
}

void BossEnemyIdleState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("backStepSpeed", &backStepSpeed_, 0.001f);
}

void BossEnemyIdleState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	backStepSpeed_ = SakuEngine::JsonAdapter::GetValue<float>(data, "backStepSpeed_");
}

void BossEnemyIdleState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["backStepSpeed_"] = backStepSpeed_;
}
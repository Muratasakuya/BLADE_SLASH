#include "BossEnemyChargeAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyChargeAttackState classMethods
//============================================================================

void BossEnemyChargeAttackState::CreateEffect() {

	// 1本の刃
	singleBlade_ = std::make_unique<BossEnemyBladeCollision>();
	singleBlade_->Init("singleBlade_Charge");
}

void BossEnemyChargeAttackState::Enter() {

	bossEnemy_->SetNextAnimation("bossEnemy_chargeAttack", false, nextAnimDuration_);

	canExit_ = false;
}

void BossEnemyChargeAttackState::UpdateAlways() {

	// 衝突更新
	singleBlade_->Update();
}

void BossEnemyChargeAttackState::Update() {

	// 常にプレイヤーの方を向くようにする
	SakuEngine::Math::RotateToDirection3D(*bossEnemy_, SakuEngine::Math::GetDirection3D(*bossEnemy_, *player_), rotationLerpRate_);
	// 刃の更新処理
	UpdateBlade();

	if (bossEnemy_->IsAnimationFinished()) {

		exitTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
		// 時間経過が過ぎたら遷移可能
		if (exitTime_ < exitTimer_) {

			canExit_ = true;
		}
	}
}

void BossEnemyChargeAttackState::UpdateBlade() {

	// キーイベントで攻撃を発生させる
	if (bossEnemy_->IsEventKey("Attack", 0)) {

		// 発生処理
		const SakuEngine::Vector3 pos = bossEnemy_->GetTranslation();
		const SakuEngine::Vector3 velocity = CalcBaseDir() * singleBladeMoveSpeed_;
		singleBlade_->EmitEffect(pos, velocity);
	}
}

SakuEngine::Vector3 BossEnemyChargeAttackState::CalcBaseDir() const {

	return (player_->GetTranslation() - bossEnemy_->GetTranslation()).Normalize();
}

void BossEnemyChargeAttackState::Exit() {

	// リセット
	exitTimer_ = 0.0f;
	canExit_ = false;
}

void BossEnemyChargeAttackState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);

	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("singleBladeMoveSpeed", &singleBladeMoveSpeed_, 0.1f);

	if (ImGui::CollapsingHeader("Blade")) {
		if (ImGui::Button("Emit SingleBlade")) {

			// 発生処理
			const SakuEngine::Vector3 pos = bossEnemy_->GetTranslation();
			const SakuEngine::Vector3 velocity = CalcBaseDir() * singleBladeMoveSpeed_;
			singleBlade_->EmitEffect(pos, velocity);
		}

		ImGui::Separator();

		singleBlade_->ImGui();
		singleBlade_->Update();
	}

	if (ImGui::CollapsingHeader("Blade Effect")) {

		ImGui::DragFloat("singleBladeScaling", &singleBladeEffectScalingValue_, 0.01f);
	}
}

void BossEnemyChargeAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");

	exitTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "exitTime_");
	singleBladeMoveSpeed_ = data.value("singleBladeMoveSpeed_", 1.0f);
	singleBladeEffectScalingValue_ = data.value("singleBladeEffectScalingValue_", 1.0f);
}

void BossEnemyChargeAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;

	data["exitTime_"] = exitTime_;
	data["singleBladeMoveSpeed_"] = singleBladeMoveSpeed_;
	data["singleBladeEffectScalingValue_"] = singleBladeEffectScalingValue_;
}
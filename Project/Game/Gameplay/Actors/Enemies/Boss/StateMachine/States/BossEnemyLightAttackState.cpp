#include "BossEnemyLightAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyLightAttackState classMethods
//============================================================================

void BossEnemyLightAttackState::CreateEffect() {

	// 剣エフェクト作成
	slash_.effect = std::make_unique<SakuEngine::EffectGroup>();
	slash_.effect->Init("lightAttackSlash", "BossEnemyEffect");
	slash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightAttackEffect_0.json");

	// 親の設定
	slash_.SetParent("bossSlash_0", bossEnemy_->GetTransform());
}

void BossEnemyLightAttackState::Enter() {

	// 攻撃予兆アニメーションを設定
	bossEnemy_->SetNextAnimation("bossEnemy_lightAttackParrySign", false, nextAnimDuration_);

	// 座標を設定
	startPos_ = bossEnemy_->GetTranslation();
	canExit_ = false;

	// 攻撃予兆を出す
	SakuEngine::Vector3 sign = bossEnemy_->GetTranslation();
	sign.y = 2.0f;
	attackSign_->Emit(SakuEngine::Math::ProjectToScreen(sign, *followCamera_));

	// パリィ可能にする
	bossEnemy_->ResetParryTiming();
	parryParam_.continuousCount = 1;
	parryParam_.canParry = true;

	parried_ = false;
}

void BossEnemyLightAttackState::Update() {

	// パリィ攻撃のタイミングを更新
	UpdateParryTiming();

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyLightAttackState::State::ParrySign: {

		// プレイヤーの方を向きながら補間
		UpdateParrySign();
		break;
	}
	case BossEnemyLightAttackState::State::Attack: {

		// 攻撃、終了後状態を終了
		UpdateAttack();
		break;
	}
	}
}

void BossEnemyLightAttackState::UpdateAlways() {

	// 剣エフェクトの更新、親の回転を設定する
	slash_.Update(*bossEnemy_);
}

void BossEnemyLightAttackState::UpdateParrySign() {

	// 再生速度の適用
	BossEnemyBaseAttackState::CheckAndApplySpeedRate();

	// 目標座標を常に更新する
	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	SakuEngine::Vector3 direction = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);
	SakuEngine::Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 0.0f;
	SakuEngine::Math::LookTarget3D(*bossEnemy_, SakuEngine::Math::GetFlattenPos3D(*player_, false), rotationLerpRate_);

	// アニメーションが終了次第攻撃する
	if (bossEnemy_->IsAnimationFinished()) {

		// 再生速度のリセット
		BossEnemyBaseAttackState::ResetSpeedRate();

		bossEnemy_->SetNextAnimation("bossEnemy_lightAttack", false, nextAnimDuration_);

		// 状態を進める
		currentState_ = State::Attack;

		// 補間をリセット
		lerpTimer_ = 0.0f;
		startPos_ = bossEnemy_->GetTranslation();
		reachedPlayer_ = false;
	}
}

void BossEnemyLightAttackState::UpdateAttack() {

	// 座標補間処理
	LerpTranslation();

	// animationが終了したら経過時間を進める
	if (bossEnemy_->IsAnimationFinished()) {

		exitTimer_ += SakuEngine::GameTimer::GetDeltaTime();
		// 時間経過が過ぎたら遷移可能
		if (exitTime_ < exitTimer_) {

			canExit_ = true;
		}
	}
}

void BossEnemyLightAttackState::UpdateParryTiming() {

	// パリィ攻撃のタイミング
	switch (currentState_) {
	case BossEnemyLightAttackState::State::Attack: {
		if (bossEnemy_->IsEventKey("Parry", 0)) {

			bossEnemy_->TellParryTiming();
			parried_ = true;
		}
		break;
	}
	}
}

void BossEnemyLightAttackState::LerpTranslation() {

	// 目標座標計算
	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	SakuEngine::Vector3 direction = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);
	SakuEngine::Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 0.0f;

	if (!reachedPlayer_) {

		// プレイヤーの方を向くようにしておく
		SakuEngine::Math::LookTarget3D(*bossEnemy_, SakuEngine::Math::GetFlattenPos3D(*player_, false), rotationLerpRate_);

		// 補間時間を進める
		lerpTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
		float lerpT = std::clamp(lerpTimer_ / lerpTime_, 0.0f, 1.0f);
		lerpT = EasedValue(easingType_, lerpT);

		// 補間
		SakuEngine::Vector3 newPos = SakuEngine::Vector3::Lerp(startPos_, target, lerpT);
		bossEnemy_->SetTranslation(newPos);

		// プレイヤーに十分近づいたら補間しない
		// xとzの距離を見る
		SakuEngine::Vector2 distanceXZ = SakuEngine::Vector2(playerPos.x - newPos.x, playerPos.z - newPos.z);
		if (distanceXZ.Length() <= std::fabs(attackOffsetTranslation_)) {

			reachedPlayer_ = true;
			bossEnemy_->SetTranslation(target);

			// 剣エフェクトの発生
			slash_.Emit(*bossEnemy_);
		}
	}
}

void BossEnemyLightAttackState::Exit() {

	// リセット
	canExit_ = false;
	reachedPlayer_ = false;
	parryParam_.canParry = false;
	lerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	currentState_ = State::ParrySign;
	bossEnemy_->ResetParryTiming();
	// 再生速度のリセット
	BossEnemyBaseAttackState::ResetSpeedRate();
}

void BossEnemyLightAttackState::ImGui() {

	ImGui::Text(std::format("parried: {}", parried_).c_str());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("lerpTime", &lerpTime_, 0.001f);
	ImGui::DragFloat("attackSpeedRate", &attackSpeedRate_, 0.01f);

	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	slash_.EditOffset("slashEffectOffset");

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text("exitTimer: %.3f", exitTimer_);
	Easing::SelectEasingType(easingType_);

	// 座標を設定
	SakuEngine::Vector3 start = bossEnemy_->GetTranslation();
	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	SakuEngine::Vector3 direction = (start - playerPos).Normalize();
	SakuEngine::Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 2.0f;
	SakuEngine::LineRenderer::GetInstance()->Get3D()->DrawSphere(8, 2.0f, target, SakuEngine::Color::Red());
}

void BossEnemyLightAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	lerpTime_ = data.value("lerpTime_", 0.16f);
	attackSpeedRate_ = data.value("attackSpeedRate_", 1.0f);

	attackOffsetTranslation_ = SakuEngine::JsonAdapter::GetValue<float>(data, "attackOffsetTranslation_");
	exitTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "exitTime_");
	easingType_ = static_cast<EasingType>(SakuEngine::JsonAdapter::GetValue<int>(data, "easingType_"));

	slash_.FromJson(data, "slashEffectOffset_");
}

void BossEnemyLightAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["lerpTime_"] = lerpTime_;
	data["attackSpeedRate_"] = attackSpeedRate_;

	data["attackOffsetTranslation_"] = attackOffsetTranslation_;
	data["exitTime_"] = exitTime_;
	data["easingType_"] = static_cast<int>(easingType_);

	slash_.ToJson(data, "slashEffectOffset_");
}
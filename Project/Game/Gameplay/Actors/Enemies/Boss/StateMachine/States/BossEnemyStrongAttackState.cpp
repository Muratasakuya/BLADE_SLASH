#include "BossEnemyStrongAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyStrongAttackState classMethods
//============================================================================

void BossEnemyStrongAttackState::CreateEffect() {

	parriedMaps_[State::Attack1st] = false;
	parriedMaps_[State::Attack2nd] = false;

	// 剣エフェクト作成
	// 強攻撃エフェクト
	strongSlash_.effect = std::make_unique<SakuEngine::EffectGroup>();
	strongSlash_.effect->Init("strongAttackSlash", "BossEnemyEffect");
	strongSlash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyStrongAttackEffect.json");
	// 弱攻撃エフェクト
	lightSlash_.effect = std::make_unique<SakuEngine::EffectGroup>();
	lightSlash_.effect->Init("lightAttackSlash", "BossEnemyEffect");
	lightSlash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightAttackEffect_1.json");

	// 親の設定
	strongSlash_.SetParent("bossSlash_1", bossEnemy_->GetTransform());
	lightSlash_.SetParent("bossSlash_0", bossEnemy_->GetTransform());
}

void BossEnemyStrongAttackState::Enter() {

	// 攻撃予兆アニメーションを設定
	bossEnemy_->SetNextAnimation("bossEnemy_strongAttackParrySign", false, nextAnimDuration_);

	// 座標を設定
	startPos_ = bossEnemy_->GetTranslation();
	canExit_ = false;

	// 攻撃予兆を出す
	SakuEngine::Vector3 sign = bossEnemy_->GetTranslation();
	sign.y = 2.0f;
	attackSign_->Emit(SakuEngine::Math::ProjectToScreen(sign, *followCamera_));

	// パリィ可能にする
	bossEnemy_->ResetParryTiming();
	parryParam_.continuousCount = 2;
	parryParam_.canParry = true;

	parriedMaps_[State::Attack1st] = false;
	parriedMaps_[State::Attack2nd] = false;
}

void BossEnemyStrongAttackState::Update() {

	// パリィ攻撃のタイミングを更新
	UpdateParryTiming();

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyStrongAttackState::State::ParrySign: {

		// プレイヤーの方を向きながら補間
		UpdateParrySign();
		break;
	}
	case BossEnemyStrongAttackState::State::Attack1st: {

		// 攻撃1回目
		UpdateAttack1st();
		break;
	}
	case BossEnemyStrongAttackState::State::Attack2nd: {

		// 攻撃2回目
		UpdateAttack2nd();
		break;
	}
	}
}

void BossEnemyStrongAttackState::UpdateAlways() {

	// エフェクト更新
	strongSlash_.Update(*bossEnemy_);
	lightSlash_.Update(*bossEnemy_);
}

void BossEnemyStrongAttackState::UpdateParrySign() {

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

		bossEnemy_->SetNextAnimation("bossEnemy_strongAttack", false, nextAnimDuration_);

		// 状態を進める
		currentState_ = State::Attack1st;

		// 座標を設定
		startPos_ = bossEnemy_->GetTranslation();

		// 剣エフェクト発生
		strongSlash_.Emit(*bossEnemy_);
	}
}

void BossEnemyStrongAttackState::UpdateAttack1st() {

	// 座標補間処理
	LerpTranslation();

	// animationが終了したら次の攻撃に移る
	if (bossEnemy_->IsAnimationFinished()) {

		bossEnemy_->SetNextAnimation("bossEnemy_lightAttack", false, attack2ndAnimDuration_);

		// 状態を進める
		currentState_ = State::Attack2nd;

		// 補間をリセット
		lerpTimer_ = 0.0f;
		startPos_ = bossEnemy_->GetTranslation();
		reachedPlayer_ = false;

		// 剣エフェクト発生
		lightSlash_.Emit(*bossEnemy_);
	}
}

void BossEnemyStrongAttackState::UpdateAttack2nd() {

	// 座標補間処理
	LerpTranslation();

	// animationが終了したら経過時間を進める
	if (bossEnemy_->IsAnimationFinished()) {

		exitTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
		// 時間経過が過ぎたら遷移可能
		if (exitTime_ < exitTimer_) {

			canExit_ = true;
		}
	}
}

void BossEnemyStrongAttackState::UpdateParryTiming() {

	// パリィ攻撃のタイミング
	switch (currentState_) {
	case BossEnemyStrongAttackState::State::Attack1st: {

		if (bossEnemy_->IsEventKey("Parry", 0)) {

			bossEnemy_->TellParryTiming();
			parriedMaps_[currentState_] = true;
		}
		break;
	}
	case BossEnemyStrongAttackState::State::Attack2nd: {

		if (bossEnemy_->IsEventKey("Parry", 0)) {

			bossEnemy_->TellParryTiming();
			parriedMaps_[currentState_] = true;
		}
		break;
	}
	}
}

void BossEnemyStrongAttackState::LerpTranslation() {

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
		float lerpT = std::clamp(lerpTimer_ / attack2ndLerpTime_, 0.0f, 1.0f);
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
		}
	}
}

void BossEnemyStrongAttackState::Exit() {

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

void BossEnemyStrongAttackState::ImGui() {

	ImGui::Text(std::format("parried: Attack1st: {}", parriedMaps_[State::Attack1st]).c_str());
	ImGui::Text(std::format("parried: Attack2nd: {}", parriedMaps_[State::Attack2nd]).c_str());
	ImGui::Text(std::format("reachedPlayer: {}", reachedPlayer_).c_str());

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("attack2ndAnimDuration", &attack2ndAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("attackSpeedRate", &attackSpeedRate_, 0.01f);

	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("attack2ndLerpTime", &attack2ndLerpTime_, 0.01f);

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text("exitTimer: %.3f", exitTimer_);
	Easing::SelectEasingType(easingType_);

	strongSlash_.EditOffset("strongSlashOffset");
	lightSlash_.EditOffset("lightSlashOffset");

	// 座標を設定
	// 座標を設定
	SakuEngine::Vector3 start = bossEnemy_->GetTranslation();
	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	SakuEngine::Vector3 direction = (start - playerPos).Normalize();
	SakuEngine::Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 2.0f;
	SakuEngine::LineRenderer::GetInstance()->DrawSphere(8, 2.0f, target, SakuEngine::Color::Red());

	ImGui::Text(std::format("(playerPos - start).Length(): {}", (playerPos - start).Length()).c_str());
}

void BossEnemyStrongAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	attack2ndAnimDuration_ = data.value("attack2ndAnimDuration_", 0.4f);
	attack2ndLerpTime_ = data.value("attack2ndLerpTime_", 0.4f);
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	attackSpeedRate_ = data.value("attackSpeedRate_", 1.0f);

	attackOffsetTranslation_ = SakuEngine::JsonAdapter::GetValue<float>(data, "attackOffsetTranslation_");
	exitTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "exitTime_");
	easingType_ = static_cast<EasingType>(SakuEngine::JsonAdapter::GetValue<int>(data, "easingType_"));

	strongSlash_.FromJson(data, "strongSlashOffset");
	lightSlash_.FromJson(data, "lightSlashOffset");
}

void BossEnemyStrongAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["attack2ndAnimDuration_"] = attack2ndAnimDuration_;
	data["attack2ndLerpTime_"] = attack2ndLerpTime_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["attackSpeedRate_"] = attackSpeedRate_;

	data["attackOffsetTranslation_"] = attackOffsetTranslation_;
	data["exitTime_"] = exitTime_;
	data["easingType_"] = static_cast<int>(easingType_);

	strongSlash_.ToJson(data, "strongSlashOffset");
	lightSlash_.ToJson(data, "lightSlashOffset");
}
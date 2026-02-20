#include "BossEnemyContinuousAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyContinuousAttackState classMethods
//============================================================================

void BossEnemyContinuousAttackState::CreateEffect() {

	// 剣エフェクト作成
	// 1回目
	firstSlash_.effect = std::make_unique<SakuEngine::EffectGroup>();
	firstSlash_.effect->Init("continuousAttackFirstSlash", "BossEnemyEffect");
	firstSlash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyContinuousAttackFirstSlashEffect.json");
	// 2回目
	secondSlash_.effect = std::make_unique<SakuEngine::EffectGroup>();
	secondSlash_.effect->Init("continuousAttackSecondSlash", "BossEnemyEffect");
	secondSlash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyJumpAttackEffect_1.json");
	// 3回目
	thirdSlash_.effect = std::make_unique<SakuEngine::EffectGroup>();
	thirdSlash_.effect->Init("continuousAttackThirdSlash", "BossEnemyEffect");
	thirdSlash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightAttackEffect_2.json");

	// 親の設定
	firstSlash_.SetParent("bossSlash_3", bossEnemy_->GetTransform());
	secondSlash_.SetParent("bossSlash_2", bossEnemy_->GetTransform());
	thirdSlash_.SetParent("bossSlash_0", bossEnemy_->GetTransform());
}

void BossEnemyContinuousAttackState::Enter() {

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
	parryParam_.continuousCount = 3;
	parryParam_.canParry = true;
	keyEventIndex_ = 0;
	parried_ = false;
	reachedPlayer_ = false;
}

void BossEnemyContinuousAttackState::Update() {

	// パリィ攻撃のタイミングを更新
	UpdateParryTiming();
	// エフェクトイベント更新
	UpdateEffectEvent();

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyContinuousAttackState::State::ParrySign: {

		// プレイヤーの方を向きながら補間
		UpdateParrySign();
		break;
	}
	case BossEnemyContinuousAttackState::State::Attack: {

		// 攻撃、終了後状態を終了
		UpdateAttack();
		break;
	}
	}
}

void BossEnemyContinuousAttackState::UpdateAlways() {

	// エフェクト更新
	firstSlash_.Update(*bossEnemy_);
	secondSlash_.Update(*bossEnemy_);
	thirdSlash_.Update(*bossEnemy_);
}

void BossEnemyContinuousAttackState::UpdateParrySign() {

	// 再生速度の適用
	BossEnemyBaseAttackState::CheckAndApplySpeedRate();

	// 目標座標を常に更新する
	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	SakuEngine::Vector3 direction = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);
	SakuEngine::Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 0.0f;
	// 常にプレイヤーの方を向くようにする
	SakuEngine::Math::LookTarget3D(*bossEnemy_, SakuEngine::Math::GetFlattenPos3D(*player_, false), rotationLerpRate_);

	// アニメーションが終了次第攻撃する
	if (bossEnemy_->IsAnimationFinished()) {

		// 再生速度のリセット
		BossEnemyBaseAttackState::ResetSpeedRate();

		// 連続攻撃アニメーションを開始させる
		bossEnemy_->SetNextAnimation("bossEnemy_continuousAttack", false, nextAnimDuration_);

		// 補間座標を設定
		startPos_ = bossEnemy_->GetTranslation();

		// 状態を進める
		currentState_ = State::Attack;
		lerpTimer_ = 0.0f;
		reachedPlayer_ = false;
	}
}

void BossEnemyContinuousAttackState::UpdateAttack() {

	// プレイヤー座標計算
	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	SakuEngine::Vector3 direction = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);
	SakuEngine::Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 0.0f;

	if (!reachedPlayer_) {

		// プレイヤーの方を向くようにしておく
		SakuEngine::Math::LookTarget3D(*bossEnemy_, SakuEngine::Math::GetFlattenPos3D(*player_, false), rotationLerpRate_);

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
		}
	}

	// animationが終了したら経過時間を進める
	if (bossEnemy_->IsAnimationFinished()) {

		exitTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
		// 時間経過が過ぎたら遷移可能
		if (exitTime_ < exitTimer_) {

			canExit_ = true;
		}
	}
}

void BossEnemyContinuousAttackState::UpdateParryTiming() {

	// パリィ攻撃のタイミング
	if (currentState_ == State::Attack) {
		if (bossEnemy_->IsEventKey("Parry", keyEventIndex_)) {

			bossEnemy_->TellParryTiming();
			parried_ = true;

			// キーイベントを進める
			++keyEventIndex_;
			keyEventIndex_ = std::clamp(keyEventIndex_, uint32_t(0), parryParam_.continuousCount - 1);

			// もう一度近づけるようにする
			reachedPlayer_ = false;
			// 補間処理をリセット
			startPos_ = bossEnemy_->GetTranslation();
			lerpTimer_ = 0.0f;
		}
	}
}

void BossEnemyContinuousAttackState::UpdateEffectEvent() {

	// 剣エフェクト発生
	// エフェクトイベントに応じて発生0,1,2は順番
	if (emitCount_ == 0 && bossEnemy_->IsEventKey("Effect", 0)) {

		firstSlash_.Emit(*bossEnemy_);
		++emitCount_;
	}
	if (bossEnemy_->IsEventKey("Effect", 1)) {

		secondSlash_.Emit(*bossEnemy_);
	}
	if (bossEnemy_->IsEventKey("Effect", 2)) {

		thirdSlash_.Emit(*bossEnemy_);
	}
}

void BossEnemyContinuousAttackState::Exit() {

	// リセット
	canExit_ = false;
	reachedPlayer_ = false;
	parryParam_.canParry = false;
	lerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	keyEventIndex_ = 0;
	currentState_ = State::ParrySign;
	bossEnemy_->ResetParryTiming();
	emitCount_ = 0;
	// 再生速度のリセット
	BossEnemyBaseAttackState::ResetSpeedRate();
}

void BossEnemyContinuousAttackState::ImGui() {

	ImGui::Text(std::format("reachedPlayer: {}", reachedPlayer_).c_str());
	ImGui::Text(std::format("emitCount: {}", emitCount_).c_str());

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("attackSpeedRate", &attackSpeedRate_, 0.01f);

	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("lerpTime_", &lerpTime_, 0.01f);

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text(std::format("keyEventIndex: {}", keyEventIndex_).c_str());
	ImGui::Text("exitTimer: %.3f", exitTimer_);
	Easing::SelectEasingType(easingType_);

	firstSlash_.EditOffset("firstSlashOffset");
	secondSlash_.EditOffset("secondSlashOffset");
	thirdSlash_.EditOffset("thirdSlashOffset");

	// 座標を設定
	// 座標を設定
	SakuEngine::Vector3 start = bossEnemy_->GetTranslation();
	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	SakuEngine::Vector3 direction = (start - playerPos).Normalize();
	SakuEngine::Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 2.0f;
	SakuEngine::LineRenderer::GetInstance()->Get3D()->DrawSphere(8, 2.0f, target, SakuEngine::Color::Red());

	ImGui::Text(std::format("(playerPos - start).Length(): {}", (playerPos - start).Length()).c_str());
}

void BossEnemyContinuousAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	lerpTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpTime_");
	attackSpeedRate_ = data.value("attackSpeedRate_", 1.0f);

	attackOffsetTranslation_ = SakuEngine::JsonAdapter::GetValue<float>(data, "attackOffsetTranslation_");
	exitTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "exitTime_");
	easingType_ = static_cast<EasingType>(SakuEngine::JsonAdapter::GetValue<int>(data, "easingType_"));

	firstSlash_.FromJson(data, "firstSlashEffectOffset");
	secondSlash_.FromJson(data, "secondSlashEffectOffset");
	thirdSlash_.FromJson(data, "thirdSlashEffectOffset");
}

void BossEnemyContinuousAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["lerpTime_"] = lerpTime_;
	data["attackSpeedRate_"] = attackSpeedRate_;

	data["attackOffsetTranslation_"] = attackOffsetTranslation_;
	data["exitTime_"] = exitTime_;
	data["easingType_"] = static_cast<int>(easingType_);

	firstSlash_.ToJson(data, "firstSlashEffectOffset");
	secondSlash_.ToJson(data, "secondSlashEffectOffset");
	thirdSlash_.ToJson(data, "thirdSlashEffectOffset");
}
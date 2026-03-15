#include "PlayerAttack_4thState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Audio/Audio.h>
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Timer/DelayedHitstop.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerAttack_4thState classMethods
//============================================================================

void PlayerAttack_4thState::CreateEffect() {

	// エフェクト作成
	// 地割れエフェクト
	groundCrackEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	groundCrackEffect_->Init("groundCrack", "PlayerEffect");
	groundCrackEffect_->LoadJson("GameEffectGroup/Player/groundCrackEffect.json");
	groundCrackEmitted_ = false;

	// 回転エフェクト
	rotationEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	rotationEffect_->Init("rotationEffect", "PlayerEffect");
	rotationEffect_->LoadJson("GameEffectGroup/Player/playerAttack4thRotateEffect.json");

	// 親の設定
	rotationEffect_->SetParent("player4thAttackRotateSlash", player_->GetTransform());
	rotationEffect_->SetParent("player4thAttackRotateLightning", player_->GetTransform());
}

void PlayerAttack_4thState::Enter() {

	player_->SetNextAnimation("player_attack_4th", false, nextAnimDuration_);
	canExit_ = false;

	// 敵が攻撃可能範囲にいるかチェック
	SakuEngine::Vector3 playerPos = player_->GetTranslation();
	assisted_ = CheckInRange(attackPosLerpCircleRange_, SakuEngine::Math::GetDistance3D(*player_, *bossEnemy_, true, true));

	// 補間座標を設定
	if (!assisted_) {

		startPos_ = playerPos;

		SakuEngine::Vector3 forward = player_->GetTransform().GetForward();
		forward.y = 0.0f;
		forward = forward.Normalize();
		targetPos_ = startPos_ + forward * moveValue_;
	}

	// 回転補間範囲内に入っていたら
	if (CheckInRange(attackLookAtCircleRange_, SakuEngine::Math::GetDistance3D(*player_, *bossEnemy_, true, true))) {

		// カメラアニメーション開始
		followCamera_->StartPlayerActionAnim(PlayerState::Attack_4th);
	}

	// 回転エフェクトの発生
	rotationEffect_->Emit(player_->GetRotation() * rotateEffectOffset_);
}

void PlayerAttack_4thState::Update() {

	// 範囲内にいるときは敵に向かって補間させる
	if (assisted_) {

		// 座標、回転補間
		AttackAssist();

		// 攻撃が命中しそうなときのみヒットストップ
		if (!attackHitStop_.isStart &&
			attackHitStop_.startProgress <= player_->GetAnimationProgress() &&
			player_->GetCurrentAnimationName() == "player_attack_4th") {

			attackHitStop_.isStart = true;
			attackHitStop_.hitStop.Start();
		}
	} else {

		// 前に前進させる
		moveTimer_.Update();
		SakuEngine::Vector3 pos = SakuEngine::Vector3::Lerp(startPos_, targetPos_, moveTimer_.easedT_);
		player_->SetTranslation(pos);
	}

	// animationが終わったかチェック
	canExit_ = player_->IsAnimationFinished();
	// animationが終わったら時間経過を進める
	if (canExit_) {

		// 発生していないときのみ
		if (!groundCrackEmitted_) {

			// 地割れエフェクトの発生
			// Y座標は固定
			SakuEngine::Vector3 emitPos = player_->GetTranslation();
			groundCrackEffect_->Emit(emitPos);
			// 発生済みにする
			groundCrackEmitted_ = true;

			// 衝撃波SE再生
			SakuEngine::Audio::GetInstance()->PlayOneShot("playerShowWave");
		}

		// シェイク前にアニメーションを終了させる
		followCamera_->EndCameraAnim();
		exitTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
	}
}

void PlayerAttack_4thState::UpdateAlways() {

	// ヒットストップの更新
	attackHitStop_.hitStop.Update();

	// 地割れエフェクトの更新
	groundCrackEffect_->Update();
	// 回転エフェクトの更新
	// 親の回転を設定する
	SakuEngine::Quaternion offsetRotation =
		SakuEngine::Quaternion::Normalize(SakuEngine::Quaternion::EulerToQuaternion(rotateEffectOffsetRotation_));
	SakuEngine::Quaternion rotation = SakuEngine::Quaternion::Normalize(SakuEngine::Quaternion::Multiply(player_->GetRotation(), offsetRotation));

	rotationEffect_->SetParentRotation("player4thAttackRotateSlash", rotation, ParticleUpdateModuleID::Rotation);
	rotationEffect_->SetParentRotation("player4thAttackRotateLightning", rotation, ParticleUpdateModuleID::Primitive);
	rotationEffect_->Update();
}

void PlayerAttack_4thState::Exit() {

	// リセット
	attackPosLerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	moveTimer_.Reset();
	attackHitStop_.isStart = false;
	attackHitStop_.hitStop.Reset();
	groundCrackEmitted_ = false;

	// カメラアニメーション終了
	followCamera_->EndCameraAnim();
}

void PlayerAttack_4thState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);

	ImGui::DragFloat3("rotateEffectOffset", &rotateEffectOffset_.x, 0.1f);
	ImGui::DragFloat3("rotateEffectOffsetRotation", &rotateEffectOffsetRotation_.x, 0.01f);

	PlayerBaseAttackState::ImGui();

	moveTimer_.ImGui("MoveTimer");
	ImGui::DragFloat("moveValue", &moveValue_, 0.1f);

	if (ImGui::CollapsingHeader("Hitstop")) {

		ImGui::ProgressBar(player_->GetAnimationProgress());
		ImGui::DragFloat("startProgress", &attackHitStop_.startProgress, 0.01f, 0.0f, 1.0f);
		attackHitStop_.hitStop.ImGui("AttackHitStop");
	}
}

void PlayerAttack_4thState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "exitTime_");

	rotateEffectOffset_ = SakuEngine::Vector3::FromJson(data.value("rotateEffectOffset_", Json()));
	rotateEffectOffsetRotation_ = SakuEngine::Vector3::FromJson(data.value("rotateEffectOffsetRotation_", Json()));

	PlayerBaseAttackState::ApplyJson(data);

	moveTimer_.FromJson(data.value("MoveTimer", Json()));
	moveValue_ = data.value("moveValue_", 1.0f);

	attackHitStop_.startProgress = data.value("startProgress", 0.0f);
	attackHitStop_.hitStop.FromJson(data.value("hitStop", Json()));
}

void PlayerAttack_4thState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	data["rotateEffectOffset_"] = rotateEffectOffset_.ToJson();
	data["rotateEffectOffsetRotation_"] = rotateEffectOffsetRotation_.ToJson();

	PlayerBaseAttackState::SaveJson(data);

	moveTimer_.ToJson(data["MoveTimer"]);
	data["moveValue_"] = moveValue_;

	data["startProgress"] = attackHitStop_.startProgress;
	attackHitStop_.hitStop.ToJson(data["hitStop"]);
}

bool PlayerAttack_4thState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}
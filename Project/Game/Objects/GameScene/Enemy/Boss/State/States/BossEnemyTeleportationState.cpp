#include "BossEnemyTeleportationState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyTeleportationState classMethods
//============================================================================

BossEnemyTeleportationState::BossEnemyTeleportationState() {

	// 残像エフェクト初期化
	afterImageEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	afterImageEffect_->Init("teleportAfterImageEffect", "BossEnemyEffect");
	afterImageEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyTeleportAfterImageEffect.json");
}

void BossEnemyTeleportationState::Enter() {

	bossEnemy_->SetNextAnimation("bossEnemy_teleport", true, nextAnimDuration_);

	// 座標を設定
	SakuEngine::Vector3 center = player_->GetTranslation();
	center.y = 0.0f;
	const SakuEngine::Vector3 forward = followCamera_->GetTransform().GetForward();
	startPos_ = bossEnemy_->GetTranslation();
	// 弧上の座標を取得
	if (type_ == BossEnemyTeleportType::Far) {

		targetPos_ = SakuEngine::Math::RandomPointOnArcInSquare(center,
			followCamera_->GetTransform().GetForward(),
			farRadius_, halfAngle_, SakuEngine::Vector3::AnyInit(0.0f), moveClampSize_ / 2.0f);
	} else if (type_ == BossEnemyTeleportType::Near) {

		targetPos_ = SakuEngine::Math::RandomPointOnArcInSquare(center,
			followCamera_->GetTransform().GetForward(),
			nearRadius_, halfAngle_, SakuEngine::Vector3::AnyInit(0.0f), moveClampSize_ / 2.0f);
	}

	currentAlpha_ = 1.0f;
	bossEnemy_->SetAlpha(currentAlpha_);

	canExit_ = false;
}

void BossEnemyTeleportationState::Update() {

	lerpTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
	float lerpT = std::clamp(lerpTimer_ / lerpTime_, 0.0f, 1.0f);
	lerpT = EasedValue(easingType_, lerpT);

	// 座標補間
	bossEnemy_->SetTranslation(SakuEngine::Vector3::Lerp(startPos_, targetPos_, lerpT));

	// playerの方を向かせる
	SakuEngine::Vector3 playerPos = player_->GetTranslation();
	playerPos.y = 0.0f;
	LookTarget(playerPos);

	const float disappearEnd = fadeOutTime_;           // 消え終わる時間
	const float appearStart = lerpTime_ - fadeInTime_; // 現れ始める時間

	bossEnemy_->SetCastShadow(true);
	if (lerpTimer_ <= disappearEnd) {

		const float t = std::clamp(lerpTimer_ / fadeOutTime_, 0.0f, 1.0f);
		currentAlpha_ = 1.0f - t;
	} else if (lerpTimer_ >= appearStart) {

		const float t = std::clamp((lerpTimer_ - appearStart) / fadeInTime_, 0.0f, 1.0f);
		currentAlpha_ = t;
	} else {

		currentAlpha_ = 0.0f;
		bossEnemy_->SetCastShadow(false);
	}
	bossEnemy_->SetAlpha(currentAlpha_);

	// 時間経過が過ぎたら状態遷移可能にする
	if (lerpTime_ < lerpTimer_) {

		bossEnemy_->SetTranslation(targetPos_);
		canExit_ = true;
	} else {

		// 補間中は発生させる
		afterImageEffect_->Emit(bossEnemy_->GetTranslation());
	}
}

void BossEnemyTeleportationState::UpdateAlways() {

	// 残像エフェクト更新、回転を設定する
	afterImageEffect_->SetParentRotation("bossAfterImage",
		SakuEngine::Quaternion::Normalize(bossEnemy_->GetRotation()), ParticleUpdateModuleID::Rotation);
	afterImageEffect_->Update();
}

void BossEnemyTeleportationState::Exit() {

	// リセット
	canExit_ = false;
	lerpTimer_ = 0.0f;
	currentAlpha_ = 1.0f;
}

void BossEnemyTeleportationState::ImGui() {

	// テレポートの種類の名前
	const char* teleportNames[] = { "Far","Near" };
	ImGui::Text("currentTeleportType: %s", teleportNames[static_cast<int>(type_)]);

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);

	ImGui::DragFloat("farRadius:Red", &farRadius_, 0.1f);
	ImGui::DragFloat("nearRadius:Blue", &nearRadius_, 0.1f);
	ImGui::DragFloat("halfAngle", &halfAngle_, 0.1f);
	ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
	ImGui::DragFloat("fadeOutTime", &fadeOutTime_, 0.01f);
	ImGui::DragFloat("fadeInTime", &fadeInTime_, 0.01f);
	Easing::SelectEasingType(easingType_);

	SakuEngine::Vector3 center = player_->GetTranslation();
	center.y = 4.0f;
	SakuEngine::LineRenderer::GetInstance()->DrawArc(8, farRadius_, halfAngle_,
		center, followCamera_->GetTransform().GetForward(), SakuEngine::Color::Red());
	SakuEngine::LineRenderer::GetInstance()->DrawArc(8, nearRadius_, halfAngle_,
		center, followCamera_->GetTransform().GetForward(), SakuEngine::Color::Blue());
}

void BossEnemyTeleportationState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	farRadius_ = SakuEngine::JsonAdapter::GetValue<float>(data, "farRadius_");
	nearRadius_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nearRadius_");
	halfAngle_ = SakuEngine::JsonAdapter::GetValue<float>(data, "halfAngle_");
	lerpTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpTime_");
	fadeOutTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "fadeOutTime_");
	fadeInTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "fadeInTime_");
	easingType_ = static_cast<EasingType>(SakuEngine::JsonAdapter::GetValue<int>(data, "easingType_"));

	{
		Json clampData;
		if (SakuEngine::JsonAdapter::LoadCheck("GameConfig/gameConfig.json", clampData)) {

			moveClampSize_ = SakuEngine::JsonAdapter::GetValue<float>(clampData["playableArea"], "length");
		}
	}
}

void BossEnemyTeleportationState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["farRadius_"] = farRadius_;
	data["nearRadius_"] = nearRadius_;
	data["halfAngle_"] = halfAngle_;
	data["lerpTime_"] = lerpTime_;
	data["fadeOutTime_"] = fadeOutTime_;
	data["fadeInTime_"] = fadeInTime_;
	data["easingType_"] = static_cast<int>(easingType_);
}
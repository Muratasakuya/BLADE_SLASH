#include "PlayerFalterState.h"

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerFalterState classMethods
//============================================================================

void PlayerFalterState::Enter() {

	// 怯みアニメーションを再生
	player_->SetNextAnimation("player_falter", false, nextAnimDuration_);

	// 向き
	SakuEngine::Vector3 direction = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);

	//補間座標を設定
	startPos_ = player_->GetTranslation();
	targetPos_ = startPos_ + direction * moveDistance_;

	// 敵の方向を向かせる
	player_->SetRotation(SakuEngine::Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up)));

	SakuEngine::GameTimer::StartHitStop(hitStopTime_, 0.0f);

	// カメラの向きを補正させる
	followCamera_->StartLookToTarget(FollowCameraTargetType::Player,
		FollowCameraTargetType::BossEnemy, true, true, targetCameraRotateX_);

	canExit_ = false;
}

void PlayerFalterState::Update() {

	// 時間を更新
	falterTimer_.Update();
	// 座標を補間
	player_->SetTranslation(SakuEngine::Vector3::Lerp(startPos_, targetPos_, falterTimer_.easedT_));

	// 補間終了、アニメーション後状態を終了する
	if (falterTimer_.IsReached() && player_->IsAnimationFinished()) {

		canExit_ = true;
	}
}

void PlayerFalterState::Exit() {

	// リセット
	falterTimer_.Reset();
}

void PlayerFalterState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("moveDistance", &moveDistance_, 0.1f);
	ImGui::DragFloat("targetCameraRotateX", &targetCameraRotateX_, 0.01f);
	ImGui::DragFloat("hitStopTime", &hitStopTime_, 0.01f);

	falterTimer_.ImGui("FalterTimer");
}

void PlayerFalterState::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.1f);
	moveDistance_ = data.value("moveDistance_", 0.5f);
	targetCameraRotateX_ = data.value("targetCameraRotateX_", 0.5f);
	hitStopTime_ = data.value("hitStopTime_", 0.05f);

	falterTimer_.FromJson(data.value("FalterTimer", Json()));
}

void PlayerFalterState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["moveDistance_"] = moveDistance_;
	data["targetCameraRotateX_"] = targetCameraRotateX_;
	data["hitStopTime_"] = hitStopTime_;

	falterTimer_.ToJson(data["FalterTimer"]);
}
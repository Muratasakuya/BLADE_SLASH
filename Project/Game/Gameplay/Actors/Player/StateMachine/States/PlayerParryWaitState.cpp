#include "PlayerParryWaitState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/CameraEditor.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

//============================================================================
//	PlayerParryWaitState classMethods
//============================================================================

void PlayerParryWaitState::Enter() {

	// 状態遷移自体はいつでも行える状態にしておく
	canExit_ = true;

	// リセット
	lerpPos_.timer.Reset();

	// アニメーション再生
	player_->SetNextAnimation("player_parryWait", false, nextAnimDuration_);

	// 向きを取得
	SakuEngine::Vector3 direction = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);
	// 敵の方を向ける
	SakuEngine::Quaternion lookRotation = SakuEngine::Quaternion::LookRotation(
		direction, Direction::Get(Direction3D::Up));
	player_->SetRotation(SakuEngine::Quaternion::Normalize(lookRotation));

	// 補間座標を決定
	lerpPos_.start = player_->GetTranslation();
	// 敵への方向から一定距離離れた位置に設定
	lerpPos_.target = SakuEngine::Math::GetFlattenPos3D(*bossEnemy_) + direction * enemyDistance_;

	// まだパリィ用のカメラアニメーションが設定されていなければ設定する
	SakuEngine::CameraEditor* cameraEditor = SakuEngine::CameraEditor::GetInstance();
	const auto& cameraAnimName = cameraEditor->GetActiveAnimationKeyName();
	if (cameraAnimName.has_value()) {
		if (*cameraAnimName != "playerParry") {

			followCamera_->StartPlayerActionAnim(PlayerState::Parry);
		}
	} else {

		followCamera_->StartPlayerActionAnim(PlayerState::Parry);
	}
}

void PlayerParryWaitState::Update() {

	// 補間が終了したらその場で留める
	if (lerpPos_.timer.IsReached()) {
		return;
	}

	// 座標補間更新
	lerpPos_.timer.Update();
	SakuEngine::Vector3 lerpPos = SakuEngine::Vector3::Lerp(
		lerpPos_.start, lerpPos_.target, lerpPos_.timer.easedT_);
	// 座標設定
	player_->SetTranslation(lerpPos);
}

void PlayerParryWaitState::Exit() {

	// リセット
	lerpPos_.timer.Reset();
}

void PlayerParryWaitState::ImGui() {

	ImGui::DragFloat("nextAnimDuration_", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("enemyDistance_", &enemyDistance_, 0.01f);

	lerpPos_.timer.ImGui("LerpPos");
}

void PlayerParryWaitState::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.1f);
	enemyDistance_ = data.value("enemyDistance_", 2.0f);

	lerpPos_.timer.FromJson(data.value("lerpPosTimer", Json()));
}

void PlayerParryWaitState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["enemyDistance_"] = enemyDistance_;

	lerpPos_.timer.ToJson(data["lerpPosTimer"]);
}
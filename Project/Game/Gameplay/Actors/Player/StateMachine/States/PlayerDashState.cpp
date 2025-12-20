#include "PlayerDashState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

//============================================================================
//	PlayerDashState classMethods
//============================================================================

PlayerDashState::PlayerDashState(const SakuEngine::InputMapper<PlayerInputAction>* inputMapper) {

	inputMapper_ = inputMapper;
}

void PlayerDashState::Enter() {

	player_->SetNextAnimation("player_dash", false, nextAnimDuration_);

	// 加速開始
	currentState_ = State::Accel;
	accelLerp_->Start();

	if (followCamera_->IsFinishedHandoffBlend() && StateNode::GetPreviousState() != PlayerState::Parry) {

		// カメラを見やすい位置まで補間させる
		followCamera_->SetOverlayState(FollowCameraOverlayState::ReturnDefaultRotate, true);
	}
}

void PlayerDashState::Update() {

	// ダッシュ更新
	UpdateDash();
	// 回転、進行方向に向かせる
	SakuEngine::Math::RotateToDirection3D(*player_, SakuEngine::Vector3(move_.x, 0.0f, move_.z).Normalize(), rotationLerpRate_);
}

void PlayerDashState::UpdateState() {

	switch (currentState_) {
	case PlayerDashState::State::Accel: {

		// 回避中
		isAvoiding_ = true;

		// 加速させる
		accelLerp_->LerpValue(moveSpeed_);
		// 加速が終了したら次の状態に遷移
		if (accelLerp_->IsFinished()) {

			currentState_ = State::Sustain;
		}
		break;
	}
	case PlayerDashState::State::Sustain: {

		sustainTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();

		// 時間経過後減速させる
		if (sustainTime_ <= sustainTimer_) {

			// 回避終了
			isAvoiding_ = false;

			currentState_ = State::Decel;
			decelLerp_->Start();
		}
		break;
	}
	case PlayerDashState::State::Decel: {

		decelLerp_->LerpValue(moveSpeed_);
		break;
	}
	}
}

void PlayerDashState::UpdateDash() {

	// 速度の状態更新
	UpdateState();

	// 入力値取得
	SakuEngine::Vector2 inputValue(inputMapper_->GetVector(PlayerInputAction::MoveX),
		inputMapper_->GetVector(PlayerInputAction::MoveZ));
	inputValue = SakuEngine::Vector2::Normalize(inputValue);
	if (inputValue.Length() > Config::kEpsilon) {

		SakuEngine::Vector3 direction = SakuEngine::Vector3::Normalize(SakuEngine::Vector3(inputValue.x, 0.0f, inputValue.y));
		direction = SakuEngine::Vector3::TransferNormal(direction,
			SakuEngine::Quaternion::MakeRotateMatrix(followCamera_->GetTransform().rotation));
		move_ = direction * moveSpeed_;
	}
	// 特に何も入力していなくても加速状態の時は向いている方向に加速分動かして進ませる
	else {

		move_ = player_->GetTransform().GetForward() * moveSpeed_;
	}
	move_.y = 0.0f;

	// 座標を設定
	SakuEngine::Vector3 translation = player_->GetTranslation();
	translation += move_;
	player_->SetTranslation(translation);
}

void PlayerDashState::Exit() {

	// animationをリセット
	accelLerp_->Reset();
	decelLerp_->Reset();
	sustainTimer_ = 0.0f;
	move_.Init();

	// 回避終了にしておく
	isAvoiding_ = false;

	player_->ResetAnimation();
}

bool PlayerDashState::GetCanExit() const {

	if (inputMapper_->IsTriggered(PlayerInputAction::Attack)) {

		return true;
	}
	return !inputMapper_->IsPressed(PlayerInputAction::Dash);
}

void PlayerDashState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate_", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("sustainTime", &sustainTime_, 0.01f);
	if (ImGui::BeginTabBar("DashSpeedTabs")) {
		if (ImGui::BeginTabItem("Accel")) {

			accelLerp_->ImGui("accel");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Decel")) {

			decelLerp_->ImGui("decel");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PlayerDashState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	sustainTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "sustainTime_");

	accelLerp_ = std::make_unique<SakuEngine::SimpleAnimation<float>>();
	decelLerp_ = std::make_unique<SakuEngine::SimpleAnimation<float>>();
	if (data.contains("accelLerp_")) {

		accelLerp_->FromJson(data["accelLerp_"]);
	}
	if (data.contains("decelLerp_")) {

		decelLerp_->FromJson(data["decelLerp_"]);
	}
}

void PlayerDashState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	accelLerp_->ToJson(data["accelLerp_"]);
	decelLerp_->ToJson(data["decelLerp_"]);
}
#include "PlayerWalkState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Input/Input.h>
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

//============================================================================
//	PlayerWalkState classMethods
//============================================================================

PlayerWalkState::PlayerWalkState(const SakuEngine::InputMapper<PlayerInputAction>* inputMapper) {

	inputMapper_ = inputMapper;
}

void PlayerWalkState::Enter() {

	// いつでも状態遷移可能にする
	canExit_ = true;

	player_->SetNextAnimation("player_walk", true, nextAnimDuration_);
}

void PlayerWalkState::Update() {

	// 歩き更新
	UpdateWalk();
	// 回転、進行方向に向かせる
	SakuEngine::Math::RotateToDirection3D(*player_, SakuEngine::Vector3(move_.x, 0.0f, move_.z).Normalize(), rotationLerpRate_);
}

void PlayerWalkState::UpdateWalk() {

	// 現在の入力タイプ
	InputType inputType = SakuEngine::Input::GetInstance()->GetType();

	// 入力値取得
	SakuEngine::Vector2 inputValue(inputMapper_->GetVector(PlayerInputAction::MoveX, inputType, true),
		inputMapper_->GetVector(PlayerInputAction::MoveZ, inputType, true));
	inputValue = SakuEngine::Vector2::Normalize(inputValue);

	if (std::fabs(inputValue.x) > Config::kEpsilon || std::fabs(inputValue.y) > Config::kEpsilon) {

		// 入力がある場合のみ速度を計算する
		SakuEngine::Vector3 inputDirection(inputValue.x, 0.0f, inputValue.y);
		inputDirection = SakuEngine::Vector3::Normalize(inputDirection);

		SakuEngine::Matrix4x4 rotateMatrix = SakuEngine::Quaternion::MakeRotateMatrix(followCamera_->GetTransform().rotation);
		SakuEngine::Vector3 rotatedDirection = SakuEngine::Vector3::TransferNormal(inputDirection, rotateMatrix);
		rotatedDirection = SakuEngine::Vector3::Normalize(rotatedDirection);

		move_ = rotatedDirection * moveSpeed_;
	} else {

		// 入力がなければどんどん減速させる
		move_ *= moveDecay_;
		// 一定の速度以下で止まる
		if (move_.Length() < Config::kEpsilon) {
			move_.Init();
		}
	}
	move_.y = 0.0f;

	// 移動量を加算
	SakuEngine::Vector3 translation = player_->GetTranslation();
	translation += move_;
	player_->SetTranslation(translation);
}

void PlayerWalkState::Exit() {
}

void PlayerWalkState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("moveSpeed", &moveSpeed_, 0.01f);
	ImGui::DragFloat("moveDecay", &moveDecay_, 0.01f);
}

void PlayerWalkState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	moveSpeed_ = SakuEngine::JsonAdapter::GetValue<float>(data, "moveSpeed_");
	moveDecay_ = SakuEngine::JsonAdapter::GetValue<float>(data, "moveDecay_");
}

void PlayerWalkState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["moveSpeed_"] = moveSpeed_;
	data["moveDecay_"] = moveDecay_;
}
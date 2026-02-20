#include "PlayerIdleState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

//============================================================================
//	PlayerIdleState classMethods
//============================================================================

PlayerIdleState::PlayerIdleState() {
}

void PlayerIdleState::Enter() {

	canExit_ = false;
	player_->SetNextAnimation("player_idle", true, nextAnimDuration_);
}

void PlayerIdleState::Update() {

	canExit_ = true;
}

void PlayerIdleState::UpdateAlways() {
}

void PlayerIdleState::Exit() {

	canExit_ = false;
}

void PlayerIdleState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
}

void PlayerIdleState::ApplyJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.01f);
}

void PlayerIdleState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
}
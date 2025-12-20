#include "FollowCameraSwitchAllyState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FollowCameraSwitchAllyState classMethods
//============================================================================

void FollowCameraSwitchAllyState::Enter([[maybe_unused]] FollowCamera& followCamera) {

	startFovY_ = std::nullopt;
}

void FollowCameraSwitchAllyState::Update(FollowCamera& followCamera) {

	SakuEngine::Vector3 translation = followCamera.GetTransform().translation;
	SakuEngine::Vector3 offset{};

	// 補間先の座標を補完割合に応じて補間する
	interTarget_ = SakuEngine::Vector3::Lerp(interTarget_, targets_[FollowCameraTargetType::Player]->GetWorldPos(), lerpRate_);

	SakuEngine::Matrix4x4 rotateMatrix = SakuEngine::Quaternion::MakeRotateMatrix(followCamera.GetTransform().rotation);
	offset = SakuEngine::Vector3::TransferNormal(offsetTranslation_, rotateMatrix);

	// offset分座標をずらす
	translation = interTarget_ + offset;
	followCamera.SetTranslation(translation);

	if (!startFovY_.has_value()) {

		startFovY_ = followCamera.GetFovY();
	}

	// 時間経過したら補間終了
	if (lerpTime_ < lerpTimer_) {

		followCamera.SetFovY(targetFovY_);
		return;
	}

	// 画角を補間する
	lerpTimer_ += SakuEngine::GameTimer::GetDeltaTime();
	float lerpT = lerpTimer_ / lerpTime_;
	lerpT = EasedValue(lerpEasingType_, lerpT);

	float fovY = std::lerp(startFovY_.value(), targetFovY_, lerpT);
	followCamera.SetFovY(fovY);
}

void FollowCameraSwitchAllyState::Exit() {

	// リセット
	lerpTimer_ = 0.0f;
	startFovY_ = std::nullopt;
}

void FollowCameraSwitchAllyState::ImGui([[maybe_unused]] const FollowCamera& followCamera) {

	ImGui::DragFloat3("offsetTranslation", &offsetTranslation_.x, 0.1f);
	ImGui::DragFloat("targetFovY", &targetFovY_, 0.1f);
	Easing::SelectEasingType(lerpEasingType_);
	ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
	ImGui::DragFloat("lerpRate", &lerpRate_, 0.1f);
}

void FollowCameraSwitchAllyState::ApplyJson(const Json& data) {

	offsetTranslation_ = SakuEngine::JsonAdapter::ToObject<SakuEngine::Vector3>(data["offsetTranslation_"]);
	targetFovY_ = SakuEngine::JsonAdapter::GetValue<float>(data, "targetFovY_");
	lerpTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpTime_");
	lerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpRate_");
	lerpEasingType_ = static_cast<EasingType>(
		SakuEngine::JsonAdapter::GetValue<int>(data, "lerpEasingType_"));
}

void FollowCameraSwitchAllyState::SaveJson(Json& data) {

	data["offsetTranslation_"] = SakuEngine::JsonAdapter::FromObject<SakuEngine::Vector3>(offsetTranslation_);
	data["targetFovY_"] = targetFovY_;
	data["lerpTime_"] = lerpTime_;
	data["lerpRate_"] = lerpRate_;
	data["lerpEasingType_"] = static_cast<int>(lerpEasingType_);
}
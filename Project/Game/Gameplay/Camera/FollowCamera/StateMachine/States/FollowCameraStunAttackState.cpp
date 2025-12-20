#include "FollowCameraStunAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/FollowCamera.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FollowCameraStunAttackState classMethods
//============================================================================

void FollowCameraStunAttackState::Enter([[maybe_unused]] FollowCamera& followCamera) {
}

void FollowCameraStunAttackState::Update(FollowCamera& followCamera) {

	// プレイヤーに追従していく処理
	// 味方に追従していく処理
	SakuEngine::Vector3 rotation = followCamera.GetTransform().eulerRotate;
	SakuEngine::Vector3 translation = followCamera.GetTransform().translation;
	SakuEngine::Vector3 offset{};

	// 補間先の座標を補完割合に応じて補間する
	interTarget_ = SakuEngine::Vector3::Lerp(interTarget_, targets_[FollowCameraTargetType::Player]->GetWorldPos(), lerpRate_);

	SakuEngine::Matrix4x4 rotateMatrix = SakuEngine::Matrix4x4::MakeRotateMatrix(rotation);
	offset = SakuEngine::Vector3::TransferNormal(offsetTranslation_, rotateMatrix);

	// offset分座標をずらす
	translation = interTarget_ + offset;
	followCamera.SetTranslation(translation);
}

void FollowCameraStunAttackState::Exit() {

	// リセット
	canExit_ = false;
}

void FollowCameraStunAttackState::ImGui([[maybe_unused]] const FollowCamera& followCamera) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());

	ImGui::DragFloat3("offsetTranslation", &offsetTranslation_.x, 0.1f);
	ImGui::DragFloat("lerpRate", &lerpRate_, 0.01f);
}

void FollowCameraStunAttackState::ApplyJson(const Json& data) {

	lerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpRate_");
	offsetTranslation_ = SakuEngine::JsonAdapter::ToObject<SakuEngine::Vector3>(data["offsetTranslation_"]);
}

void FollowCameraStunAttackState::SaveJson(Json& data) {

	data["lerpRate_"] = lerpRate_;
	data["offsetTranslation_"] = SakuEngine::JsonAdapter::FromObject<SakuEngine::Vector3>(offsetTranslation_);
}
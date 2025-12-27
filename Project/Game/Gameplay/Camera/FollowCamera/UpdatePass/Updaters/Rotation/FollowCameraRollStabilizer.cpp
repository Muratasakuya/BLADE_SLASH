#include "FollowCameraRollStabilizer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FollowCameraRollStabilizer classMethods
//============================================================================

void FollowCameraRollStabilizer::Init() {

	// json適用
	ApplyJson();
}

void FollowCameraRollStabilizer::Execute(FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, float deltaTime) {

	// 現在の回転を取得
	SakuEngine::Quaternion candidate = context.cameraRotation;

	// 現在の前方ベクトルと上方向ベクトルから、ロールを安定化させた回転を計算
	SakuEngine::Vector3 forward = candidate * Direction::Get(Direction3D::Forward);
	SakuEngine::Quaternion upright = SakuEngine::Quaternion::LookRotation(forward.Normalize(), Direction::Get(Direction3D::Up));

	// カメラ回転を補間してロールを安定化(Z軸回転を0.0fに戻す)
	float t = std::clamp(lerpRate_ * deltaTime, 0.0f, 1.0f);
	context.cameraRotation = SakuEngine::Quaternion::Slerp(candidate, upright, t);
}

void FollowCameraRollStabilizer::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("lerpRate", &lerpRate_, 0.01f);
}

void FollowCameraRollStabilizer::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/rollStabilizer.json", data)) {
		return;
	}

	lerpRate_ = data.value("lerpRate_", 0.8f);
}

void FollowCameraRollStabilizer::SaveJson() {

	Json data;

	data["lerpRate_"] = lerpRate_;

	SakuEngine::JsonAdapter::Save("Camera/Follow/rollStabilizer.json", data);
}
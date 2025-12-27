#include "FollowCameraZoomOffsetResolver.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Rotation/FollowCameraPitchClamper.h>

//============================================================================
//	FollowCameraZoomOffsetResolver classMethods
//============================================================================

void FollowCameraZoomOffsetResolver::Init() {

	// json適用
	ApplyJson();
}

void FollowCameraZoomOffsetResolver::Execute(FollowCameraContext& context,
	const FollowCameraFrameService& service, [[maybe_unused]] float deltaTime) {

	// 制限済みの回転からpitchを取り直す
	SakuEngine::Vector3 forward = context.cameraRotation * Direction::Get(Direction3D::Forward);
	float pitch = std::atan2(forward.y, std::sqrt(forward.x * forward.x + forward.z * forward.z));

	// 最小、最大ピッチ角度からの距離を計算
	float distanceToMin = std::fabs(pitch - service.pitchClamper->GetMinPitch());
	float distanceToMax = std::fabs(pitch - service.pitchClamper->GetMaxPitch());

	// デフォルトのZオフセットから目標Zオフセットを計算
	float targetZ = defaultOffset_.z;
	if (distanceToMin < rotateMinusParam_.clampThreshold) {

		rotateMinusParam_.CalTargetOffsetZ(distanceToMin, defaultOffset_.z, targetZ);
	} else if (distanceToMax < rotatePlusParam_.clampThreshold) {

		rotatePlusParam_.CalTargetOffsetZ(distanceToMax, defaultOffset_.z, targetZ);
	}
	// 目標オフセットを設定
	desiredOffset_ = defaultOffset_;
	desiredOffset_.z = targetZ;
}

void FollowCameraZoomOffsetResolver::RotateParam::CalTargetOffsetZ(
	float distance, float baseTargetZ, float& outTargetZ) const {

	// 閾値外なら補間しない
	if (clampThreshold <= distance) {
		return;
	}

	// 閾値に近づくほどoffsetZNearに補間
	float t = 1.0f - (distance / clampThreshold);
	t = std::clamp(t, 0.0f, 1.0f);
	outTargetZ = std::lerp(baseTargetZ, offsetZNear, t);
}

void FollowCameraZoomOffsetResolver::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Config");

	ImGui::Text("DesiredOffset: (%.2f / %.2f / %.2f)", desiredOffset_.x, desiredOffset_.y, desiredOffset_.z);

	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat3("defaultOffset", &defaultOffset_.x, 0.01f);

	rotatePlusParam_.ImGui("PlusParam");
	rotateMinusParam_.ImGui("MinusParam");
}

void FollowCameraZoomOffsetResolver::RotateParam::ImGui(const char* label) {

	ImGui::SeparatorText(label);
	ImGui::PushID(label);

	ImGui::DragFloat("rotateClampX", &rotateClampX, 0.01f);
	ImGui::DragFloat("offsetZNear", &offsetZNear, 0.01f);
	ImGui::DragFloat("clampThreshold", &clampThreshold, 0.01f);

	ImGui::PopID();
}

void FollowCameraZoomOffsetResolver::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/zoomOffsetResolver.json", data)) {
		return;
	}

	defaultOffset_ = SakuEngine::Vector3::FromJson(data.value("defaultOffset_", Json()));
	rotatePlusParam_.FromJson(data.value("rotatePlusParam_", Json()));
	rotateMinusParam_.FromJson(data.value("rotateMinusParam_", Json()));
}

void FollowCameraZoomOffsetResolver::SaveJson() {

	Json data;

	data["defaultOffset_"] = defaultOffset_.ToJson();
	rotatePlusParam_.ToJson(data["rotatePlusParam_"]);
	rotateMinusParam_.ToJson(data["rotateMinusParam_"]);

	SakuEngine::JsonAdapter::Save("Camera/Follow/zoomOffsetResolver.json", data);
}

void FollowCameraZoomOffsetResolver::RotateParam::FromJson(const Json& data) {

	rotateClampX = data.value("rotateClampX", 0.0f);
	offsetZNear = data.value("offsetZNear", 0.0f);
	clampThreshold = data.value("clampThreshold", 0.0f);
}

void FollowCameraZoomOffsetResolver::RotateParam::ToJson(Json& data) {

	data["rotateClampX"] = rotateClampX;
	data["offsetZNear"] = offsetZNear;
	data["clampThreshold"] = clampThreshold;
}
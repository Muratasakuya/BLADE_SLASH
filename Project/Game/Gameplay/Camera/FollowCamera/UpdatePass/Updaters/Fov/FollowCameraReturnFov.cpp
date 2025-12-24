#include "FollowCameraReturnFov.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FollowCameraReturnFov classMethods
//============================================================================

void FollowCameraReturnFov::Init() {

	// json適用
	ApplyJson();
}

void FollowCameraReturnFov::Execute(FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, float deltaTime) {

	// 画角をデフォルトの値にフレーム補間する
	float t = std::clamp(lerpRate_ * deltaTime, 0.0f, 1.0f);
	context.cameraFovY = std::lerp(context.cameraFovY, defaultFovY_, t);
}

void FollowCameraReturnFov::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("lerpRate", &lerpRate_, 0.01f);
	ImGui::DragFloat("defaultFovY", &defaultFovY_, 0.01f);
}

void FollowCameraReturnFov::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/returnFov.json", data)) {
		return;
	}

	lerpRate_ = data.value("lerpRate_", 0.8f);
	defaultFovY_ = data.value("defaultFovY_", 60.0f);
}

void FollowCameraReturnFov::SaveJson() {

	Json data;

	data["lerpRate_"] = lerpRate_;
	data["defaultFovY_"] = defaultFovY_;

	SakuEngine::JsonAdapter::Save("Camera/Follow/returnFov.json", data);
}
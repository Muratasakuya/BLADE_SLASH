#include "FollowCameraInterTargetSmoother.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Target/FollowCameraTargetResolver.h>

//============================================================================
//	FollowCameraInterTargetSmoother classMethods
//============================================================================

void FollowCameraInterTargetSmoother::Init() {

	// json適用
	ApplyJson();
}

void FollowCameraInterTargetSmoother::Execute(FollowCameraContext& context,
	const FollowCameraFrameService& service, float deltaTime) {

	// 追従位置を目標位置にフレーム補間する
	float t = std::clamp(lerpRate_ * deltaTime, 0.0f, 1.0f);
	context.interTarget = SakuEngine::Vector3::Lerp(context.interTarget, service.targetResolver->GetTargetPos(), t);
}

void FollowCameraInterTargetSmoother::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("lerpRate", &lerpRate_, 0.01f);
}

void FollowCameraInterTargetSmoother::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/interTargetSmoother.json", data)) {
		return;
	}

	lerpRate_ = data.value("lerpRate_", 0.8f);
}

void FollowCameraInterTargetSmoother::SaveJson() {

	Json data;

	data["lerpRate_"] = lerpRate_;

	SakuEngine::JsonAdapter::Save("Camera/Follow/interTargetSmoother.json", data);
}
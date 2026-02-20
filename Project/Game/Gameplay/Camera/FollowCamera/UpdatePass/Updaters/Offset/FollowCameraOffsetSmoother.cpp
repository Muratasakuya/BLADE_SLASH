#include "FollowCameraOffsetSmoother.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Offset/FollowCameraZoomOffsetResolver.h>

//============================================================================
//	FollowCameraOffsetSmoother classMethods
//============================================================================

void FollowCameraOffsetSmoother::Init() {

	// 初期化
	initialized_ = false;

	// json適用
	ApplyJson();
}

void FollowCameraOffsetSmoother::Execute([[maybe_unused]] FollowCameraContext& context,
	const FollowCameraFrameService& service, float deltaTime) {

	// 目標オフセット距離を取得
	SakuEngine::Vector3 desired = service.zoomOffsetResolver->GetDesiredOffset();

	// 未初期化なら補間せずに直で代入
	if (!initialized_) {
		currentOffset_ = desired;
		initialized_ = true;
		return;
	}

	// 現在のオフセット距離を目標オフセット距離にフレーム補間する
	currentOffset_ = SakuEngine::Vector3::Lerp(currentOffset_, desired, lerpRate_ * deltaTime);
}

void FollowCameraOffsetSmoother::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Config");

	ImGui::Text(std::format("Initialized: {}", initialized_).c_str());
	ImGui::Text("CurrentOffset: (%.2f / %.2f / %.2f)", currentOffset_.x, currentOffset_.y, currentOffset_.z);

	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat3("lerpRate", &lerpRate_.x, 0.01f);
}

void FollowCameraOffsetSmoother::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/offsetSmoother.json", data)) {
		return;
	}

	lerpRate_ = SakuEngine::Vector3::FromJson(data.value("lerpRate_", Json()));
}

void FollowCameraOffsetSmoother::SaveJson() {

	Json data;

	data["lerpRate_"] = lerpRate_.ToJson();

	SakuEngine::JsonAdapter::Save("Camera/Follow/offsetSmoother.json", data);
}
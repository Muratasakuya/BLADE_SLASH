#include "FollowCameraPitchClamper.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Rotation/FollowCameraLookRotationIntegrator.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Editor/FollowCameraReturnToFollowSmoother.h>

//============================================================================
//	FollowCameraPitchClamper classMethods
//============================================================================

void FollowCameraPitchClamper::Init() {

	// json適用
	ApplyJson();
}

void FollowCameraPitchClamper::Execute(FollowCameraContext& context,
	const FollowCameraFrameService& service, [[maybe_unused]] float deltaTime) {

	// ブレンド処理中はピッチ制限を行わない
	if (service.toFollowSmoother->IsBlending()) {
		return;
	}

	// 現在の回転を取得
	Quaternion rotation = context.cameraRotation;

	// 前方ベクトルからピッチ角度を計算
	Vector3 forward = rotation * Direction::Get(Direction3D::Forward);
	float pitch = std::atan2(forward.y, std::sqrt(forward.x * forward.x + forward.z * forward.z));

	// ピッチ角度を制限
	float clamped = std::clamp(pitch, minPitch_, maxPitch_);
	// ピッチ角度が制限内であれば処理しない
	if (clamped == pitch) {
		return;
	}

	// 制限後のピッチ角度で回転を再計算
	Vector3 horizontal(forward.x, 0.0f, forward.z);
	if (horizontal.Length() < Config::kEpsilon) {

		Vector3 forwardYaw = service.rotationIntegrator->GetYawRotation() * Direction::Get(Direction3D::Forward);
		horizontal = Vector3(forwardYaw.x, 0.0f, forwardYaw.z);
	}
	horizontal = horizontal.Normalize();

	// 新しい前方ベクトルを計算
	Vector3 forwardClamped(horizontal.x * std::cos(clamped), std::sin(clamped), horizontal.z * std::cos(clamped));
	// 回転を適用
	context.cameraRotation = Quaternion::LookRotation(forwardClamped.Normalize(), Direction::Get(Direction3D::Up));
}

void FollowCameraPitchClamper::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("minPitch", &minPitch_, 0.01f);
	ImGui::DragFloat("maxPitch", &maxPitch_, 0.01f);
}

void FollowCameraPitchClamper::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/pitchClamper.json", data)) {
		return;
	}

	minPitch_ = data.value("minPitch_", -1.5f);
	maxPitch_ = data.value("maxPitch_", 1.5f);
}

void FollowCameraPitchClamper::SaveJson() {

	Json data;

	data["minPitch_"] = minPitch_;
	data["maxPitch_"] = maxPitch_;

	SakuEngine::JsonAdapter::Save("Camera/Follow/pitchClamper.json", data);
}
#include "FollowCameraLookRotationIntegrator.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Input/FollowCameraLookInputSmoother.h>

//============================================================================
//	FollowCameraLookRotationIntegrator classMethods
//============================================================================

void FollowCameraLookRotationIntegrator::Init() {

	// 初期化
	yawRotation_ = Quaternion::Identity();
}

void FollowCameraLookRotationIntegrator::Execute(FollowCameraContext& context,
	const FollowCameraFrameService& service, [[maybe_unused]] float deltaTime) {

	// 入力補間クラスから1フレーム分の回転量を取得
	Vector2 delta = service.inputSmoother->GetFrameRotationDelta();
	// 現在の回転を取得
	Quaternion rotation = context.cameraRotation;

	// ヨー回転を更新
	yawRotation_ = Quaternion::Normalize(Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Up), delta.x) * rotation);
	// ピッチ回転を計算
	Vector3 rightAxis = (yawRotation_ * Direction::Get(Direction3D::Right)).Normalize();
	Quaternion pitchRotation = Quaternion::Normalize(Quaternion::MakeAxisAngle(rightAxis, delta.y));

	// 回転を適用
	context.cameraRotation = Quaternion::Normalize(pitchRotation * yawRotation_);
}

void FollowCameraLookRotationIntegrator::ImGui() {

	ImGui::SeparatorText("Config");

	ImGui::Text("YawRotation: (%.2f / %.2f / %.2f / %.2f)",
		yawRotation_.x, yawRotation_.y, yawRotation_.z, yawRotation_.w);
}
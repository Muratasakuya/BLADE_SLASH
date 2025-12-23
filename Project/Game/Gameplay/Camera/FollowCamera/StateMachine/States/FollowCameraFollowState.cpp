#include "FollowCameraFollowState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Input/Input.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

// inputDevice
#include <Game/Gameplay/Camera/FollowCamera/Input/Devices/FollowCameraKeyInput.h>
#include <Game/Gameplay/Camera/FollowCamera/Input/Devices/FollowCameraGamePadInput.h>

//============================================================================
//	FollowCameraFollowState classMethods
//============================================================================

FollowCameraFollowState::FollowCameraFollowState() {

	// 入力クラスを初期化
	inputSmoother_ = std::make_unique<FollowCameraLookInputSmoother>();
	inputSmoother_->Init();

	// 画角を元に戻すクラスを初期化
	returnFov_ = std::make_unique<FollowCameraReturnFov>();
	returnFov_->Init();
}

void FollowCameraFollowState::SnapToCamera() {

	// 補間位置を初期化
	interTarget_ = anchorObject_->GetTranslation();
	const auto& transform = followCamera_->GetTransform();
	offsetTranslation_ = SakuEngine::Quaternion::Conjugate(
		SakuEngine::Quaternion::Normalize(transform.rotation)) * (transform.translation - interTarget_);
	// 現在位置のオフセットを記録
	handoffDefault_ = offsetTranslation_;

	// 初期化
	handoffBlendT_ = 0.0f;
	clampBlendT_ = 0.0f;
}

void FollowCameraFollowState::UpdateInitialSettings() {

	// 回転を考慮したオフセットと追従先の座標を足す
	SakuEngine::Vector3 translation = anchorObject_->GetTranslation() +
		followCamera_->GetTransform().rotation * offsetTranslation_;
	interTarget_ = translation;
	followCamera_->SetTranslation(translation);
}

void FollowCameraFollowState::Enter() {
}

void FollowCameraFollowState::Update() {

	// deltaTime取得
	float deltaTime = SakuEngine::GameTimer::GetDeltaTime();

	// コンテキストとサービス作成
	// コンテキスト
	FollowCameraContext context{};
	context.cameraFovY = followCamera_->GetFovY();
	context.cameraTranslation = followCamera_->GetTransform().translation;
	context.cameraRotation = followCamera_->GetTransform().rotation;
	// サービス
	FollowCameraFrameService service{};
	service.inputSmoother = inputSmoother_.get();
	service.returnFov = returnFov_.get();

	// 入力補間更新
	inputSmoother_->Execute(context, service, deltaTime);
	const SakuEngine::Vector2& frameRotationDelta = inputSmoother_->GetFrameRotationDelta();
	// 画角補間更新
	returnFov_->Execute(context, service, deltaTime);
	followCamera_->SetFovY(context.cameraFovY);

	// 追従ターゲット位置の補間
	{
		float t = std::clamp(lerpRate_ * deltaTime, 0.0f, 1.0f);
		interTarget_ = SakuEngine::Vector3::Lerp(interTarget_, anchorObject_->GetTranslation(), t);
	}

	SakuEngine::Quaternion currentRotation = followCamera_->GetTransform().rotation;
	// Y軸の回転
	SakuEngine::Quaternion yawRotation = SakuEngine::Quaternion::Normalize(SakuEngine::Quaternion::MakeAxisAngle(
		Direction::Get(Direction3D::Up), frameRotationDelta.x) * currentRotation);
	// X軸の回転
	SakuEngine::Vector3 rightAxis = (yawRotation * Direction::Get(Direction3D::Right)).Normalize();
	SakuEngine::Quaternion pitchRotation = SakuEngine::Quaternion::Normalize(
		SakuEngine::Quaternion::MakeAxisAngle(rightAxis, frameRotationDelta.y));
	// X軸回転とY軸回転を合成
	SakuEngine::Quaternion candidateRotation = SakuEngine::Quaternion::Normalize(pitchRotation * yawRotation);

	// 前方ベクトルからX軸回転を取得
	SakuEngine::Vector3 forward = candidateRotation * Direction::Get(Direction3D::Forward);
	float currentPitch = std::atan2(forward.y, std::sqrt(forward.x * forward.x + forward.z * forward.z));
	// X軸の回転を制御
	float pitchClamped = std::clamp(currentPitch,
		rotateMinusParam_.rotateClampX, rotatePlusParam_.rotateClampX);
	// 回転をクランプしたら回転を再設定する
	if (pitchClamped != currentPitch) {

		SakuEngine::Vector3 horizontal = SakuEngine::Vector3(forward.x, 0.0f, forward.z);
		if (horizontal.Length() < Config::kEpsilon) {

			SakuEngine::Vector3 fowrardYaw = yawRotation * Direction::Get(Direction3D::Forward);
			horizontal = SakuEngine::Vector3(fowrardYaw.x, 0.0f, fowrardYaw.z);
		}
		horizontal = horizontal.Normalize();

		SakuEngine::Vector3 forwardClamped = SakuEngine::Vector3(horizontal.x * std::cos(pitchClamped),
			std::sin(pitchClamped), horizontal.z * std::cos(pitchClamped)).Normalize();
		candidateRotation = SakuEngine::Quaternion::LookRotation(forwardClamped, Direction::Get(Direction3D::Up));
	}

	// Z回転を常に0.0fに補間
	SakuEngine::Vector3 forwardRoll = candidateRotation * Direction::Get(Direction3D::Forward);
	SakuEngine::Quaternion currentRoll = SakuEngine::Quaternion::LookRotation(forwardRoll.Normalize(), Direction::Get(Direction3D::Up));

	SakuEngine::Quaternion rotation = SakuEngine::Quaternion::Identity();
	// 回転を補間して決定する
	{
		float t = std::clamp(rotateZLerpRate_ * deltaTime, 0.0f, 1.0f);
		rotation = SakuEngine::Quaternion::Slerp(candidateRotation, currentRoll, t);
	}

	// 補間処理後クランプされるまでの値を補間
	clampBlendT_ = (std::min)(1.0f, clampBlendT_ + clampBlendSpeed_ * deltaTime);
	handoffBlendT_ = (std::min)(1.0f, handoffBlendT_ + handoffBlendSpeed_ * deltaTime);
	// 補間処理後にデフォルトのオフセットになるように補間する
	SakuEngine::Vector3 baseDefaultOffset = SakuEngine::Vector3::Lerp(handoffDefault_, defaultOffset_, handoffBlendT_);

	// クランプされるまでの距離
	float distanceToMinus = std::fabs(pitchClamped - rotateMinusParam_.rotateClampX);
	float distanceToPlus = std::fabs(pitchClamped - rotatePlusParam_.rotateClampX);
	float targetZ = baseDefaultOffset.z;
	// 距離が閾値以下なら補間値分最大/最小の回転に近づける
	if (distanceToMinus < rotateMinusParam_.clampThreshold) {

		float t = 1.0f - (distanceToMinus / rotateMinusParam_.clampThreshold);
		targetZ = std::lerp(baseDefaultOffset.z, rotateMinusParam_.offsetZNear, t);
	} else if (distanceToPlus < rotatePlusParam_.clampThreshold) {

		float t = 1.0f - (distanceToPlus / rotatePlusParam_.clampThreshold);
		targetZ = std::lerp(baseDefaultOffset.z, rotatePlusParam_.offsetZNear, t);
	}

	// オフセットの補間
	float blendedTargetZ = std::lerp(offsetTranslation_.z, targetZ, clampBlendT_);
	{
		// 各軸ごとに補間
		float tX = std::clamp(offsetLerpRate_.x * deltaTime, 0.0f, 1.0f);
		float tY = std::clamp(offsetLerpRate_.y * deltaTime, 0.0f, 1.0f);
		float tZ = std::clamp(offsetLerpRate_.z * deltaTime, 0.0f, 1.0f);
		offsetTranslation_.x = std::lerp(offsetTranslation_.x, baseDefaultOffset.x, tX);
		offsetTranslation_.y = std::lerp(offsetTranslation_.y, baseDefaultOffset.y, tY);
		offsetTranslation_.z = std::lerp(offsetTranslation_.z, blendedTargetZ, tZ);
	}

	// 回転を考慮したオフセットと追従先の座標を足す
	SakuEngine::Vector3 translation = interTarget_ + rotation * offsetTranslation_;

	// カメラにセット
	followCamera_->SetRotation(rotation);
	followCamera_->SetTranslation(translation);
}

void FollowCameraFollowState::Exit() {
}

void FollowCameraFollowState::ImGui() {

	if (ImGui::CollapsingHeader("Input")) {

		inputSmoother_->ImGui();
	}
	if (ImGui::CollapsingHeader("ReturnFov")) {

		returnFov_->ImGui();
	}
	if (ImGui::CollapsingHeader("Before")) {

		ImGui::Text("clampBlendT: %.3f", clampBlendT_);
		ImGui::Text("handoffBlendT: %.3f", handoffBlendT_);

		ImGui::DragFloat3("offsetTranslation", &offsetTranslation_.x, 0.1f);
		ImGui::DragFloat("lerpRate", &lerpRate_, 0.01f);

		ImGui::DragFloat("clampBlendSpeed", &clampBlendSpeed_, 0.01f);
		ImGui::DragFloat("handoffBlendSpeed", &handoffBlendSpeed_, 0.01f);

		ImGui::DragFloat("offsetLerpRate", &offsetLerpRate_.x, 0.001f);
		ImGui::DragFloat("rotateZLerpRate", &rotateZLerpRate_, 0.001f);
		ImGui::DragFloat("rotatePlusParam.rotateClampX", &rotatePlusParam_.rotateClampX, 0.001f);
		ImGui::DragFloat("rotatePlusParam.offsetZNear", &rotatePlusParam_.offsetZNear, 0.001f);
		ImGui::DragFloat("rotatePlusParam.clampThreshold", &rotatePlusParam_.clampThreshold, 0.001f);

		ImGui::DragFloat("rotateMinusParam.rotateClampX", &rotateMinusParam_.rotateClampX, 0.001f);
		ImGui::DragFloat("rotateMinusParam.offsetZNear", &rotateMinusParam_.offsetZNear, 0.001f);
		ImGui::DragFloat("rotateMinusParam.clampThreshold", &rotateMinusParam_.clampThreshold, 0.001f);

		ImGui::Checkbox("isDrawDebugLine", &isDrawDebugLine_);
		ImGui::DragFloat("lookAtDistanceRate", &lookAtDistanceRate_, 0.001f, 0.0f, 1.0f);

		if (!isDrawDebugLine_) {
			return;
		}

		// デバッグ用ライン描画
		{
			SakuEngine::LineRenderer* lineRenderer = SakuEngine::LineRenderer::GetInstance();

			// 基準点
			SakuEngine::Vector3 anchorPos = anchorObject_->GetTranslation();
			lineRenderer->DrawSphere(6, 2.0f, anchorPos, SakuEngine::Color::White());
			// 注視点
			SakuEngine::Vector3 lookAtPos = lookAtTargetObject_->GetTranslation();
			lineRenderer->DrawSphere(6, 2.0f, lookAtPos, SakuEngine::Color::White());
			// 間をつなぐ
			lineRenderer->DrawLine3D(anchorPos, lookAtPos, SakuEngine::Color::White());

			// 距離割合に応じた位置を取得する
			SakuEngine::Vector3 targetPos = SakuEngine::Vector3::Lerp(anchorPos, lookAtPos, lookAtDistanceRate_);
			lineRenderer->DrawSphere(6, 2.0f, targetPos, SakuEngine::Color::Red());
		}
	}
}

void FollowCameraFollowState::ApplyJson(const Json& data) {

	offsetTranslation_ = SakuEngine::JsonAdapter::ToObject<SakuEngine::Vector3>(data["offsetTranslation_"]);
	defaultOffset_ = offsetTranslation_;
	offsetTranslation_.x = 0.0f;
	// 現在位置のオフセットを記録
	handoffDefault_ = offsetTranslation_;

	lerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpRate_");
	offsetLerpRate_.x = SakuEngine::JsonAdapter::GetValue<float>(data, "offsetXLerpRate_");
	offsetLerpRate_.y = SakuEngine::JsonAdapter::GetValue<float>(data, "offsetYLerpRate_");
	offsetLerpRate_.z = SakuEngine::JsonAdapter::GetValue<float>(data, "offsetZLerpRate_");
	rotateZLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotateZLerpRate_");

	rotatePlusParam_.rotateClampX = SakuEngine::JsonAdapter::GetValue<float>(data, "rotateClampPlusX_");
	rotatePlusParam_.offsetZNear = SakuEngine::JsonAdapter::GetValue<float>(data, "rotatePlusParam_.offsetZNear");
	rotatePlusParam_.clampThreshold = SakuEngine::JsonAdapter::GetValue<float>(data, "rotatePlusParam_.clampThreshold");

	rotateMinusParam_.rotateClampX = SakuEngine::JsonAdapter::GetValue<float>(data, "rotateClampMinusX_");
	rotateMinusParam_.offsetZNear = SakuEngine::JsonAdapter::GetValue<float>(data, "rotateMinusParam_.offsetZNear");
	rotateMinusParam_.clampThreshold = SakuEngine::JsonAdapter::GetValue<float>(data, "rotateMinusParam_.clampThreshold");

	clampBlendSpeed_ = data.value("clampBlendSpeed_", 0.08f);
	handoffBlendSpeed_ = data.value("handoffBlendSpeed_", 0.08f);
}

void FollowCameraFollowState::SaveJson(Json& data) {

	data["offsetTranslation_"] = SakuEngine::JsonAdapter::FromObject<SakuEngine::Vector3>(offsetTranslation_);
	data["lerpRate_"] = lerpRate_;

	data["offsetXLerpRate_"] = offsetLerpRate_.x;
	data["offsetZLerpRate_"] = offsetLerpRate_.z;
	data["offsetYLerpRate_"] = offsetLerpRate_.y;
	data["rotateZLerpRate_"] = rotateZLerpRate_;
	data["clampBlendSpeed_"] = clampBlendSpeed_;
	data["handoffBlendSpeed_"] = handoffBlendSpeed_;

	data["rotateClampPlusX_"] = rotatePlusParam_.rotateClampX;
	data["rotatePlusParam_.offsetZNear"] = rotatePlusParam_.offsetZNear;
	data["rotatePlusParam_.clampThreshold"] = rotatePlusParam_.clampThreshold;

	data["rotateClampMinusX_"] = rotateMinusParam_.rotateClampX;
	data["rotateMinusParam_.offsetZNear"] = rotateMinusParam_.offsetZNear;
	data["rotateMinusParam_.clampThreshold"] = rotateMinusParam_.clampThreshold;
}

void FollowCameraFollowState::SetOffsetTranslation(const SakuEngine::Vector3& translation) {

	offsetTranslation_ = translation;
}

bool FollowCameraFollowState::IsFinishedHandoffBlend() const {

	bool result = (handoffBlendT_ >= 1.0f);
	result |= (clampBlendT_ >= 1.0f);
	return result;
}
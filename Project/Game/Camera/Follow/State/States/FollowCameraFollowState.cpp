#include "FollowCameraFollowState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>

//============================================================================
//	FollowCameraFollowState classMethods
//============================================================================

void FollowCameraFollowState::SnapToCamera(const FollowCamera& camera) {

	// 補間位置を初期化
	interTarget_ = targets_[FollowCameraTargetType::Player]->translation;
	const auto& transform = camera.GetTransform();
	offsetTranslation_ = SakuEngine::Quaternion::Conjugate(
		SakuEngine::Quaternion::Normalize(transform.rotation)) * (transform.translation - interTarget_);
	// 現在位置のオフセットを記録
	handoffDefault_ = offsetTranslation_;

	// 初期化
	handoffBlendT_ = 0.0f;
	clampBlendT_ = 0.0f;
	smoothedInput_ = SakuEngine::Vector2::AnyInit(0.0f);
}

void FollowCameraFollowState::UpdateInitialSettings(FollowCamera& followCamera) {

	// 画角
	followCamera.SetFovY(defaultFovY_);

	// 回転を考慮したオフセットと追従先の座標を足す
	SakuEngine::Vector3 translation = targets_[FollowCameraTargetType::Player]->translation +
		followCamera.GetTransform().rotation * offsetTranslation_;
	interTarget_ = translation;
	followCamera.SetTranslation(translation);
}

void FollowCameraFollowState::Enter([[maybe_unused]] FollowCamera& followCamera) {
}

void FollowCameraFollowState::Update(FollowCamera& followCamera) {

	// deltaTime取得
	float deltaTime = SakuEngine::GameTimer::GetDeltaTime();

	// 画角に変更があっても常に初期値に補間させる
	{
		float t = std::clamp(fovYLerpRate_ * deltaTime, 0.0f, 1.0f);
		float fovY = std::lerp(followCamera.GetFovY(), defaultFovY_, t);
		followCamera.SetFovY(fovY);
	}

	// 追従ターゲット位置の補間
	{
		float t = std::clamp(lerpRate_ * deltaTime, 0.0f, 1.0f);
		interTarget_ = SakuEngine::Vector3::Lerp(interTarget_, targets_[FollowCameraTargetType::Player]->translation, t);
	}

	// 入力から移動量を取得する
	InputType inputType = SakuEngine::Input::GetInstance()->GetType();
	SakuEngine::Vector2 rawInput(inputMapper_->GetVector(FollowCameraInputAction::RotateX), inputMapper_->GetVector(FollowCameraInputAction::RotateY));
	// 入力が合ったらブレンド処理を終了させる
	if (rawInput.Length() > std::numeric_limits<float>::epsilon()) {

		handoffBlendT_ = 1.0f;
		clampBlendT_ = 1.0f;
		handoffDefault_ = defaultOffset_;
	}

	// 入力補間を適応する
	{
		float t = std::clamp(inputLerpRate_ * deltaTime, 0.0f, 1.0f);
		smoothedInput_ = SakuEngine::Vector2::Lerp(smoothedInput_, rawInput, t);
	}

	// X軸とY軸の入力量
	// Y軸
	float yawSpeed = (inputType == InputType::Keyboard) ? mouseSensitivity_.y : padSensitivity_.y;
	// X軸
	float pitchSpeed = (inputType == InputType::Keyboard) ? mouseSensitivity_.x : padSensitivity_.x;
	// 1フレーム分の回転量
	float yawDelta = smoothedInput_.x * yawSpeed * deltaTime;
	float pitchDelta = -smoothedInput_.y * pitchSpeed * deltaTime;

	SakuEngine::Quaternion currentRotation = followCamera.GetTransform().rotation;
	// Y軸の回転
	SakuEngine::Quaternion yawRotation = SakuEngine::Quaternion::Normalize(SakuEngine::Quaternion::MakeAxisAngle(
		Direction::Get(Direction3D::Up), yawDelta) * currentRotation);
	// X軸の回転
	SakuEngine::Vector3 rightAxis = (yawRotation * Direction::Get(Direction3D::Right)).Normalize();
	SakuEngine::Quaternion pitchRotation = SakuEngine::Quaternion::Normalize(
		SakuEngine::Quaternion::MakeAxisAngle(rightAxis, pitchDelta));
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
		if (horizontal.Length() < std::numeric_limits<float>::epsilon()) {

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
	followCamera.SetRotation(rotation);
	followCamera.SetTranslation(translation);
}

void FollowCameraFollowState::Exit() {
}

void FollowCameraFollowState::ImGui([[maybe_unused]] const FollowCamera& followCamera) {

	ImGui::Text("clampBlendT: %.3f", clampBlendT_);
	ImGui::Text("handoffBlendT: %.3f", handoffBlendT_);

	ImGui::DragFloat3("offsetTranslation", &offsetTranslation_.x, 0.1f);

	ImGui::DragFloat("lerpRate", &lerpRate_, 0.01f);
	ImGui::DragFloat("inputLerpRate_", &inputLerpRate_, 0.01f);

	ImGui::DragFloat("clampBlendSpeed", &clampBlendSpeed_, 0.01f);
	ImGui::DragFloat("handoffBlendSpeed", &handoffBlendSpeed_, 0.01f);
	ImGui::DragFloat2("mouseSensitivity", &mouseSensitivity_.x, 0.001f);
	ImGui::DragFloat2("padSensitivity", &padSensitivity_.x, 0.001f);
	ImGui::DragFloat2("smoothedInput", &smoothedInput_.x, 0.001f);

	ImGui::DragFloat("fovYLerpRate", &fovYLerpRate_, 0.001f);
	ImGui::DragFloat("offsetLerpRate", &offsetLerpRate_.x, 0.001f);
	ImGui::DragFloat("rotateZLerpRate", &rotateZLerpRate_, 0.001f);
	ImGui::DragFloat("rotatePlusParam.rotateClampX", &rotatePlusParam_.rotateClampX, 0.001f);
	ImGui::DragFloat("rotatePlusParam.offsetZNear", &rotatePlusParam_.offsetZNear, 0.001f);
	ImGui::DragFloat("rotatePlusParam.clampThreshold", &rotatePlusParam_.clampThreshold, 0.001f);

	ImGui::DragFloat("rotateMinusParam.rotateClampX", &rotateMinusParam_.rotateClampX, 0.001f);
	ImGui::DragFloat("rotateMinusParam.offsetZNear", &rotateMinusParam_.offsetZNear, 0.001f);
	ImGui::DragFloat("rotateMinusParam.clampThreshold", &rotateMinusParam_.clampThreshold, 0.001f);
}

void FollowCameraFollowState::ApplyJson(const Json& data) {

	offsetTranslation_ = SakuEngine::JsonAdapter::ToObject<SakuEngine::Vector3>(data["offsetTranslation_"]);
	defaultOffset_ = offsetTranslation_;
	offsetTranslation_.x = 0.0f;
	// 現在位置のオフセットを記録
	handoffDefault_ = offsetTranslation_;

	lerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpRate_");
	inputLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "inputLerpRate_");
	mouseSensitivity_ = SakuEngine::JsonAdapter::ToObject<SakuEngine::Vector2>(data["mouseSensitivity_"]);
	padSensitivity_ = SakuEngine::JsonAdapter::ToObject<SakuEngine::Vector2>(data["padSensitivity_"]);

	fovYLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "fovYLerpRate_");
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
	data["inputLerpRate_"] = inputLerpRate_;
	data["mouseSensitivity_"] = SakuEngine::JsonAdapter::FromObject<SakuEngine::Vector2>(mouseSensitivity_);
	data["padSensitivity_"] = SakuEngine::JsonAdapter::FromObject<SakuEngine::Vector2>(padSensitivity_);

	data["fovYLerpRate_"] = fovYLerpRate_;
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
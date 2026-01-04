#include "FollowCameraReturnToFollowSmoother.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <cmath>

#include <Engine/Config.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

//============================================================================
//	FollowCameraReturnToFollowSmoother classMethods
//============================================================================

void FollowCameraReturnToFollowSmoother::Init() {

	// json適用
	ApplyJson();
}

void FollowCameraReturnToFollowSmoother::BeginBlendFromPose(const Vector3& translation,
	const Quaternion& rotation, float fovY) {

	// ブレンド処理開始
	isBlending_ = true;
	blendTimer_ = 0.0f;

	// ブレンド開始値を設定
	blendedPos_ = translation;
	blendedRotate_ = Quaternion::Normalize(rotation);
	blendedFovY_ = fovY;
}

void FollowCameraReturnToFollowSmoother::BeginBlendFromCurrentCamera() {

	// 現在のカメラ姿勢を起点にする
	const auto& transform = dependencies_.camera->GetTransform();
	BeginBlendFromPose(transform.translation, transform.rotation, dependencies_.camera->GetFovY());
}

void FollowCameraReturnToFollowSmoother::CheckEndEditorUpdate(FollowCameraContext& context) {

	// 依存が無ければ処理できない
	if (!dependencies_.camera) {
		wasInEditorUpdate_ = false;
		return;
	}

	// エディター更新フラグの変化を検知する
	bool current = dependencies_.camera->IsUpdateEditor();

	// trueからfalseに変化した瞬間にブレンド処理を開始
	if (wasInEditorUpdate_ && !current) {

		BeginBlendFromCurrentCamera();
	}
	wasInEditorUpdate_ = current;
}

void FollowCameraReturnToFollowSmoother::Execute(FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, float deltaTime) {

	// エディター更新の終了を検知して必要ならブレンド開始
	CheckEndEditorUpdate(context);

	// ブレンド中以外は何もしない
	if (!isBlending_) {
		return;
	}
	if (dependencies_.camera && dependencies_.camera->IsUpdateEditor()) {
		return;
	}

	// 時間を進める
	blendTimer_ += deltaTime;

	// 最大時間を超えたら強制終了
	if (maxBlendTime_ <= blendTimer_) {
		isBlending_ = false;
		blendTimer_ = 0.0f;
		return;
	}

	// このフレームの追従最終目標
	Vector3 desiredPos = context.cameraTranslation;
	Quaternion desiredRotate = Quaternion::Normalize(context.cameraRotation);
	{
		Quaternion rotation = Quaternion::Normalize(desiredRotate);
		// X軸回転成分だけ抽出
		Quaternion twistX = Quaternion::ExtractTwistX(rotation);
		// Y、Z成分側の回転
		Quaternion swing = Quaternion::Multiply(rotation, Quaternion::Inverse(twistX));

		// 差し替えるX軸回転
		Quaternion fixedTwistX = Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Right), pitchRotateX_);
		// 合成して正規化
		desiredRotate = Quaternion::Normalize(Quaternion::Multiply(swing, fixedTwistX));
	}
	// 画角
	float desiredFovY = context.cameraFovY;

	// 補間率を算出
	float posAlpha = HalfLifeToAlpha(posHalfLife_, deltaTime);
	float rotateAlpha = HalfLifeToAlpha(rotateHalfLife_, deltaTime);
	float fovAlpha = HalfLifeToAlpha(fovHalfLife_, deltaTime);

	// 目標へ寄せる
	blendedPos_ = Vector3::Lerp(blendedPos_, desiredPos, posAlpha);
	blendedRotate_ = Quaternion::Slerp(blendedRotate_, desiredRotate, rotateAlpha);
	blendedFovY_ = std::lerp(blendedFovY_, desiredFovY, fovAlpha);

	// 出力を上書きする
	context.cameraTranslation = blendedPos_;
	context.cameraRotation = blendedRotate_;
	context.cameraFovY = blendedFovY_;

	// 終了判定、閾値に一定以上近づいたら終了
	float posDiff = (desiredPos - blendedPos_).Length();
	float rotateDiffDeg = Math::QuaternionAngleDeg(blendedRotate_, desiredRotate);
	float fovDiff = std::fabs(desiredFovY - blendedFovY_);
	if (posDiff <= finishPosEpsilon_ &&
		rotateDiffDeg <= finishRotateEpsilonDeg_ &&
		fovDiff <= finishFovEpsilon_) {

		isBlending_ = false;
		blendTimer_ = 0.0f;
	}
}

float FollowCameraReturnToFollowSmoother::HalfLifeToAlpha(float halfLife, float deltaTime) {

	// 0以下なら早期リターンで1.0fを返す
	if (halfLife <= 0.0f) {
		return 1.0f;
	}

	// 半減期から補間率を計算
	float lambda = std::log(2.0f) / halfLife;
	return 1.0f - std::exp(-lambda * deltaTime);
}

void FollowCameraReturnToFollowSmoother::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::SeparatorText("Runtime");

	ImGui::Text(std::format("isBlending: {}", isBlending_).c_str());
	ImGui::Text(std::format("blendTimer: {:.3f}", blendTimer_).c_str());
	ImGui::Text(std::format("wasInEditorUpdate: {}", wasInEditorUpdate_).c_str());

	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("posHalfLife", &posHalfLife_, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("rotateHalfLife", &rotateHalfLife_, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("fovHalfLife", &fovHalfLife_, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("maxBlendTime", &maxBlendTime_, 0.01f, 0.0f, 60.0f);
	ImGui::DragFloat("pitchRotateX", &pitchRotateX_, 0.001f);

	ImGui::SeparatorText("Finish Threshold");

	ImGui::DragFloat("finishPosEpsilon", &finishPosEpsilon_, 0.001f, 0.0f, 10.0f);
	ImGui::DragFloat("finishRotateEpsilonDeg", &finishRotateEpsilonDeg_, 0.01f, 0.0f, 180.0f);
	ImGui::DragFloat("finishFovEpsilon", &finishFovEpsilon_, 0.001f, 0.0f, 180.0f);
}

void FollowCameraReturnToFollowSmoother::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Follow/returnToFollowSmoother.json", data)) {
		return;
	}

	posHalfLife_ = data.value("posHalfLife_", posHalfLife_);
	rotateHalfLife_ = data.value("rotateHalfLife_", rotateHalfLife_);
	fovHalfLife_ = data.value("fovHalfLife_", fovHalfLife_);
	maxBlendTime_ = data.value("maxBlendTime_", maxBlendTime_);
	pitchRotateX_ = data.value("pitchRotateX_", pitchRotateX_);

	finishPosEpsilon_ = data.value("finishPosEpsilon_", finishPosEpsilon_);
	finishRotateEpsilonDeg_ = data.value("finishRotateEpsilonDeg_", finishRotateEpsilonDeg_);
	finishFovEpsilon_ = data.value("finishFovEpsilon_", finishFovEpsilon_);
}

void FollowCameraReturnToFollowSmoother::SaveJson() {

	Json data;

	data["posHalfLife_"] = posHalfLife_;
	data["rotateHalfLife_"] = rotateHalfLife_;
	data["fovHalfLife_"] = fovHalfLife_;
	data["maxBlendTime_"] = maxBlendTime_;
	data["pitchRotateX_"] = pitchRotateX_;

	data["finishPosEpsilon_"] = finishPosEpsilon_;
	data["finishRotateEpsilonDeg_"] = finishRotateEpsilonDeg_;
	data["finishFovEpsilon_"] = finishFovEpsilon_;

	JsonAdapter::Save("Camera/Follow/returnToFollowSmoother.json", data);
}
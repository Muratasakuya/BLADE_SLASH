#include "FollowCameraReturnToFollowSmoother.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

//============================================================================
//	FollowCameraReturnToFollowSmoother classMethods
//============================================================================

void FollowCameraReturnToFollowSmoother::Init() {

	// 初期化
	isReturning_ = false;
	justStartedReturn_ = false;
	returnTimer_.Reset();
	returnTimer_.target_ = duration_;
	returnTimer_.easeingType_ = easingType_;
	pitchRotateX_ = 0.0f;

	// json適用
	ApplyJson();
}

void FollowCameraReturnToFollowSmoother::CheckEndEditorUpdate(FollowCameraContext& context) {

	// 現在のエディター更新フラグ
	bool current = dependencies_.camera->IsUpdateEditor();

	// エディター更新中は戻しブレンドしない
	if (current) {
		isReturning_ = false;
		justStartedReturn_ = false;
		wasInEditorUpdate_ = true;
		return;
	}

	// trueからfalseに変化した瞬間にブレンド処理を開始
	if (wasInEditorUpdate_ && !current) {

		StartReturn(context);
	}
	wasInEditorUpdate_ = current;
}

void FollowCameraReturnToFollowSmoother::StartReturn(FollowCameraContext& context) {

	// 開始姿勢の取得
	startTranslation_ = dependencies_.camera->GetTransform().translation;
	startRotation_ = Quaternion::Normalize(dependencies_.camera->GetTransform().rotation);
	startFovY_ = dependencies_.camera->GetFovY();
	startInterTarget_ = context.interTarget;

	// 追従基準からのワールドオフセット
	startWorldOffset_ = startTranslation_ - startInterTarget_;

	// 既に十分近いなら戻し処理を開始しない
	Vector3 desiredTranslation = context.cameraTranslation;
	Quaternion desiredRotation = Quaternion::Normalize(context.cameraRotation);
	float desiredFovY = context.cameraFovY;

	float posDistance = (desiredTranslation - startTranslation_).Length();
	float fovDistance = std::fabs(desiredFovY - startFovY_);

	// クォータニオン内積から角度を計算
	float dot = Quaternion::Dot(startRotation_, desiredRotation);
	dot = std::fabs(dot);
	dot = std::clamp(dot, 0.0f, 1.0f);
	const float angleRad = 2.0f * std::acos(dot);

	// 十分近いなら戻し処理を開始しない
	if (posDistance < startPosThreshold_ && angleRad < startAngleThresholdRad_ && fovDistance < startFovThreshold_) {
		isReturning_ = false;
		return;
	}

	// 戻しブレンド開始
	isReturning_ = true;
	justStartedReturn_ = true;
	returnTimer_.Reset();
	returnTimer_.target_ = (std::max)(duration_, Config::kEpsilon);
	returnTimer_.easeingType_ = easingType_;
}

void FollowCameraReturnToFollowSmoother::ApplyReturnBlend(FollowCameraContext& context) {

	// 目標通常追従の結果を取得
	Vector3 desiredTranslation = context.cameraTranslation;
	Quaternion desiredRotation = Quaternion::Normalize(context.cameraRotation);
	// ピッチを固定にする
	{
		Quaternion rotation = Quaternion::Normalize(desiredRotation);
		// X軸回転成分だけ抽出 
		Quaternion twistX = Quaternion::ExtractTwistX(rotation);
		// Y、Z成分側の回転
		Quaternion swing = Quaternion::Multiply(rotation, Quaternion::Inverse(twistX));
		// 差し替えるX軸回転
		Quaternion fixedTwistX = Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Right), pitchRotateX_);
		// 合成して正規化 
		desiredRotation = Quaternion::Normalize(Quaternion::Multiply(swing, fixedTwistX));
	}
	float desiredFovY = context.cameraFovY;

	// 現在の追従基準からのオフセットをブレンド
	Vector3 desiredWorldOffset = desiredTranslation - context.interTarget;
	Vector3 blendedWorldOffset = Vector3::Lerp(startWorldOffset_, desiredWorldOffset, returnTimer_.easedT_);

	// 姿勢ブレンド
	context.cameraRotation = Quaternion::Slerp(startRotation_, desiredRotation, returnTimer_.easedT_);
	context.cameraTranslation = context.interTarget + blendedWorldOffset;
	context.cameraFovY = std::lerp(startFovY_, desiredFovY, returnTimer_.easedT_);

	// 終了処理
	if (returnTimer_.IsReached()) {

		// 目標にスナップ
		context.cameraTranslation = desiredTranslation;
		context.cameraRotation = desiredRotation;
		context.cameraFovY = desiredFovY;
		isReturning_ = false;
	}
}

void FollowCameraReturnToFollowSmoother::Execute(FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, [[maybe_unused]] float deltaTime) {

	// エディター更新の終了を検知して必要ならブレンド開始
	CheckEndEditorUpdate(context);

	// 戻しブレンド
	if (isReturning_) {
		// 開始フレームはエディター最終姿勢をそのまま適用する
		if (justStartedReturn_) {

			justStartedReturn_ = false;
			ApplyReturnBlend(context);
		} else {

			// ブレンド更新
			returnTimer_.Update();
			ApplyReturnBlend(context);
		}
	}
}

void FollowCameraReturnToFollowSmoother::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::SeparatorText("Runtime");

	ImGui::Text(std::format("wasInEditorUpdate: {}", wasInEditorUpdate_).c_str());
	ImGui::Text(std::format("isReturning: {}", isReturning_).c_str());
	ImGui::Text("returnT: %.3f", returnTimer_.easedT_);
	ImGui::Text("startTranslation: (%.2f / %.2f / %.2f)", startTranslation_.x, startTranslation_.y, startTranslation_.z);
	ImGui::Text("startInterTarget: (%.2f / %.2f / %.2f)", startInterTarget_.x, startInterTarget_.y, startInterTarget_.z);
	ImGui::Text("startWorldOffset: (%.2f / %.2f / %.2f)", startWorldOffset_.x, startWorldOffset_.y, startWorldOffset_.z);

	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("durationSec", &duration_, 0.01f);
	ImGui::DragFloat("startPosThreshold", &startPosThreshold_, 0.001f);
	ImGui::DragFloat("startAngleThresholdRad", &startAngleThresholdRad_, 0.001f);
	ImGui::DragFloat("startFovThreshold", &startFovThreshold_, 0.001f);
	ImGui::DragFloat("pitchRotateX", &pitchRotateX_, 0.001f);
	Easing::SelectEasingType(easingType_);
}

void FollowCameraReturnToFollowSmoother::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Follow/returnToFollowSmoother.json", data)) {
		return;
	}

	// duration
	duration_ = data.value("durationSec_", duration_);

	if (data.contains("easingType_")) {
		if (data["easingType_"].is_string()) {
			auto opt = EnumAdapter<EasingType>::FromString(data["easingType_"]);
			if (opt.has_value()) {
				easingType_ = opt.value();
			}
		} else if (data["easingType_"].is_number_integer()) {
			easingType_ = static_cast<EasingType>(data["easingType_"].get<int>());
		}
	}

	startPosThreshold_ = data.value("startPosThreshold_", startPosThreshold_);
	startAngleThresholdRad_ = data.value("startAngleThresholdRad_", startAngleThresholdRad_);
	startFovThreshold_ = data.value("startFovThreshold_", startFovThreshold_);
	pitchRotateX_ = data.value("pitchRotateX_", pitchRotateX_);
}

void FollowCameraReturnToFollowSmoother::SaveJson() {

	Json data;

	data["durationSec_"] = duration_;
	data["easingType_"] = EnumAdapter<EasingType>::ToString(easingType_);
	data["startPosThreshold_"] = startPosThreshold_;
	data["startAngleThresholdRad_"] = startAngleThresholdRad_;
	data["startFovThreshold_"] = startFovThreshold_;
	data["pitchRotateX_"] = pitchRotateX_;

	JsonAdapter::Save("Camera/Follow/returnToFollowSmoother.json", data);
}
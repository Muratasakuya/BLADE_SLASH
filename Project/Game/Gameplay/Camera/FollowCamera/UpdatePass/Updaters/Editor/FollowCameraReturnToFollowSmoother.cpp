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
	returnTimer_.target_ = durationSec_;
	returnTimer_.easeingType_ = easingType_;

	// json適用
	ApplyJson();
}

void FollowCameraReturnToFollowSmoother::CheckEndEditorUpdate(
	FollowCameraContext& context, const FollowCameraFrameService& service) {

	// 依存が無ければ処理できない
	if (!dependencies_.camera) {
		wasInEditorUpdate_ = false;
		isReturning_ = false;
		justStartedReturn_ = false;
		return;
	}

	// 現在のエディター更新フラグ
	bool current = dependencies_.camera->IsUpdateEditor();

	// エディター更新中は戻しブレンドしない(次に抜けたら開始)
	if (current) {
		isReturning_ = false;
		justStartedReturn_ = false;
		wasInEditorUpdate_ = true;
		return;
	}

	// trueからfalseに変化した瞬間にブレンド処理を開始
	if (wasInEditorUpdate_ && !current) {
		StartReturn(context, service);
	}

	wasInEditorUpdate_ = current;
}

void FollowCameraReturnToFollowSmoother::StartReturn(
	FollowCameraContext& context, const FollowCameraFrameService& service) {
	(void)service;

	// 開始姿勢(エディターの最終フレーム)
	// BindEndEditCameraPose() が呼ばれていないケースもあるので、Transform優先で取得
	startTranslation_ = dependencies_.camera->GetTransform().translation;
	startRotation_ = SakuEngine::Quaternion::Normalize(dependencies_.camera->GetTransform().rotation);
	startFovY_ = dependencies_.camera->GetFovY();

	// 追従基準(interTarget)は直前の追従更新から維持されている値を使う。
	// ※ここを強制的にプレイヤー座標へスナップすると、既に計算済みの desiredTranslation と
	//   interTarget の整合性が崩れて 1フレーム目のブレンドが歪みやすい。
	//   (旧FollowStateではSnapToCamera後に一連の計算を行っていたが、UpdatePass分割後は
	//   このパスが最後に実行されるため)
	startInterTarget_ = context.interTarget;

	// 追従基準からのワールドオフセット
	startWorldOffset_ = startTranslation_ - startInterTarget_;

	// 既に十分近いなら戻しを開始しない
	const SakuEngine::Vector3 desiredTranslation = context.cameraTranslation;
	const SakuEngine::Quaternion desiredRotation = SakuEngine::Quaternion::Normalize(context.cameraRotation);
	const float desiredFovY = context.cameraFovY;

	const float posDist = (desiredTranslation - startTranslation_).Length();
	const float fovDist = std::fabs(desiredFovY - startFovY_);

	// クォータニオン内積から角度(ラジアン)を計算
	float dot = startRotation_.x * desiredRotation.x + startRotation_.y * desiredRotation.y +
		startRotation_.z * desiredRotation.z + startRotation_.w * desiredRotation.w;
	dot = std::fabs(dot);
	dot = std::clamp(dot, 0.0f, 1.0f);
	const float angleRad = 2.0f * std::acos(dot);

	if (posDist < startPosThreshold_ && angleRad < startAngleThresholdRad_ && fovDist < startFovThreshold_) {
		isReturning_ = false;
		return;
	}

	// 戻しブレンド開始
	isReturning_ = true;
	justStartedReturn_ = true;
	returnTimer_.Reset();
	returnTimer_.target_ = (std::max)(durationSec_, Config::kEpsilon);
	returnTimer_.easeingType_ = easingType_;
}

void FollowCameraReturnToFollowSmoother::ApplyReturnBlend(FollowCameraContext& context) {

	// 目標(通常追従の結果)を取得
	const SakuEngine::Vector3 desiredTranslation = context.cameraTranslation;
	const SakuEngine::Quaternion desiredRotation = SakuEngine::Quaternion::Normalize(context.cameraRotation);
	const float desiredFovY = context.cameraFovY;

	// 現在の追従基準からのオフセット(ワールド)をブレンド
	// ※ローカル空間オフセット同士を直接Lerpすると基準回転が異なるため、ワールドで補間する
	const SakuEngine::Vector3 desiredWorldOffset = desiredTranslation - context.interTarget;
	const SakuEngine::Vector3 blendedWorldOffset =
		SakuEngine::Vector3::Lerp(startWorldOffset_, desiredWorldOffset, returnTimer_.easedT_);

	// 姿勢ブレンド
	context.cameraRotation = SakuEngine::Quaternion::Slerp(startRotation_, desiredRotation, returnTimer_.easedT_);
	context.cameraTranslation = context.interTarget + blendedWorldOffset;
	context.cameraFovY = std::lerp(startFovY_, desiredFovY, returnTimer_.easedT_);

	// 終了処理
	if (returnTimer_.IsReached()) {

		// 目標にスナップ(誤差を消す)
		context.cameraTranslation = desiredTranslation;
		context.cameraRotation = desiredRotation;
		context.cameraFovY = desiredFovY;
		isReturning_ = false;
	}
}

void FollowCameraReturnToFollowSmoother::Execute(FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, float deltaTime) {

	(void)deltaTime;

	// エディター更新の終了を検知して必要ならブレンド開始
	CheckEndEditorUpdate(context, service);

	// 戻しブレンド
	if (isReturning_) {
		// 開始フレームは editor 最終姿勢をそのまま出したいので、t=0のまま適用する
		if (justStartedReturn_) {
			justStartedReturn_ = false;
			ApplyReturnBlend(context);
		} else {
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

	ImGui::DragFloat("durationSec", &durationSec_, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("startPosThreshold", &startPosThreshold_, 0.001f, 0.0f, 10.0f);
	ImGui::DragFloat("startAngleThresholdRad", &startAngleThresholdRad_, 0.0001f, 0.0f, 3.14f);
	ImGui::DragFloat("startFovThreshold", &startFovThreshold_, 0.0001f, 0.0f, 10.0f);
	Easing::SelectEasingType(easingType_);
}

void FollowCameraReturnToFollowSmoother::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Follow/returnToFollowSmoother.json", data)) {
		return;
	}

	// duration
	durationSec_ = data.value("durationSec_", durationSec_);

	// easingType
	// 文字列/数値のどちらでも読めるようにする
	if (data.contains("easingType_")) {
		if (data["easingType_"].is_string()) {
			auto opt = SakuEngine::EnumAdapter<EasingType>::FromString(data["easingType_"]);
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
}

void FollowCameraReturnToFollowSmoother::SaveJson() {

	Json data;

	data["durationSec_"] = durationSec_;
	data["easingType_"] = SakuEngine::EnumAdapter<EasingType>::ToString(easingType_);
	data["startPosThreshold_"] = startPosThreshold_;
	data["startAngleThresholdRad_"] = startAngleThresholdRad_;
	data["startFovThreshold_"] = startFovThreshold_;

	JsonAdapter::Save("Camera/Follow/returnToFollowSmoother.json", data);
}
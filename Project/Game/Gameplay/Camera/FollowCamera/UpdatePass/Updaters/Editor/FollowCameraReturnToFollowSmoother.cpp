#include "FollowCameraReturnToFollowSmoother.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Editor/Camera/CameraEditor.h>
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

	// 処理を行う対象のキーかどうかチェック
	CameraEditor* cameraEditor = CameraEditor::GetInstance();
	bool isFoundKey = false;
	for (const auto& keyName : targetKeyNames_) {
		// 処理するキーが存在するので続行
		if (cameraEditor->GetLastActiveKeyName() == keyName) {
			isFoundKey = true;
			break;
		}
	}
	// 対象キーでなければ処理しない
	if (!isFoundKey) {
		isReturning_ = false;
		return;
	}

	// 開始姿勢の取得
	startTranslation_ = dependencies_.camera->GetTransform().translation;
	startRotation_ = Quaternion::Normalize(dependencies_.camera->GetTransform().rotation);
	startFovY_ = dependencies_.camera->GetFovY();
	startInterTarget_ = context.interTarget;

	// 追従基準からのワールドオフセット
	startWorldOffset_ = startTranslation_ - startInterTarget_;

	// 戻しブレンド開始
	isReturning_ = true;
	justStartedReturn_ = true;
	returnTimer_.Reset();
	returnTimer_.target_ = (std::max)(duration_, Config::kEpsilon);
	returnTimer_.easeingType_ = easingType_;
}

void FollowCameraReturnToFollowSmoother::ApplyReturnBlend(FollowCameraContext& context) {

	Vector3 desiredTranslation = context.cameraTranslation;
	float desiredFovY = context.cameraFovY;

	Quaternion desiredRotationOriginal = Quaternion::Normalize(context.cameraRotation);

	// X軸回転補正適用
	Quaternion desiredRotationFixed = desiredRotationOriginal;
	{
		Quaternion twistX = Quaternion::ExtractTwistX(desiredRotationOriginal);
		Quaternion swing = Quaternion::Multiply(desiredRotationOriginal, Quaternion::Inverse(twistX));
		Quaternion fixedTwistX = Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Right), pitchRotateX_);
		desiredRotationFixed = Quaternion::Normalize(Quaternion::Multiply(swing, fixedTwistX));
	}


	// 補正後の回転でのワールドオフセット計算
	Vector3 desiredWorldOffsetOriginal = desiredTranslation - context.interTarget;
	Vector3 localOffset = Quaternion::RotateVector(desiredWorldOffsetOriginal,
		Quaternion::Normalize(Quaternion::Inverse(desiredRotationOriginal)));

	// 補正後回転でのワールドオフセット
	Vector3 desiredWorldOffsetFixed = Quaternion::RotateVector(localOffset, desiredRotationFixed);
	Vector3 desiredTranslationFixed = context.interTarget + desiredWorldOffsetFixed;

	// オフセットのブレンド適用
	Vector3 blendedWorldOffset = Vector3::Lerp(startWorldOffset_, desiredWorldOffsetFixed, returnTimer_.easedT_);

	// カメラコンテキストに渡す
	context.cameraRotation = Quaternion::Normalize(Quaternion::Slerp(startRotation_, desiredRotationFixed, returnTimer_.easedT_));
	context.cameraTranslation = context.interTarget + blendedWorldOffset;
	context.cameraFovY = std::lerp(startFovY_, desiredFovY, returnTimer_.easedT_);

	// 終了処理
	if (returnTimer_.IsReached()) {

		// 目標にスナップ
		context.cameraTranslation = desiredTranslation;
		context.cameraRotation = desiredRotationFixed;
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
	ImGui::DragFloat("pitchRotateX", &pitchRotateX_, 0.001f);
	Easing::SelectEasingType(easingType_);

	ImGui::SeparatorText("Target Keys");

	CameraEditor* cameraEditor = CameraEditor::GetInstance();
	std::vector<std::string> availableKeys;
	if (cameraEditor) {
		availableKeys = cameraEditor->GetKeyObjectNames();
	}

	// 選択インデックスの範囲補正
	if (availableKeys.empty()) {
		imguiAddKeyIndex_ = -1;
		imguiAddKeyName_.clear();
	} else {
		imguiAddKeyIndex_ = std::clamp<int32_t>(imguiAddKeyIndex_, 0, static_cast<int32_t>(availableKeys.size() - 1));
		if (imguiAddKeyName_.empty() && imguiAddKeyIndex_ >= 0) {
			imguiAddKeyName_ = availableKeys[imguiAddKeyIndex_];
		}
	}

	// 追加UI
	{
		// CameraEditorに存在するキー名から選択
		int index = static_cast<int>(imguiAddKeyIndex_);
		if (ImGuiHelper::ComboFromStrings("Available Key", &index, availableKeys, 12)) {

			imguiAddKeyIndex_ = static_cast<int32_t>(index);
			if (imguiAddKeyIndex_ >= 0 && imguiAddKeyIndex_ < static_cast<int32_t>(availableKeys.size())) {
				imguiAddKeyName_ = availableKeys[imguiAddKeyIndex_];
			} else {
				imguiAddKeyName_.clear();
			}
		}

		ImGui::SameLine();
		const bool canAdd = !imguiAddKeyName_.empty();
		if (!canAdd) {
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button("Add##TargetKey")) {
			const bool already =
				(std::find(targetKeyNames_.begin(), targetKeyNames_.end(), imguiAddKeyName_) != targetKeyNames_.end());
			if (!already) {
				targetKeyNames_.emplace_back(imguiAddKeyName_);
			}
		}
		if (!canAdd) {
			ImGui::PopStyleVar();
		}

		ImGui::SameLine();
		if (ImGui::Button("Clear##TargetKey")) {
			targetKeyNames_.clear();
		}
	}

	// 現在の対象キー一覧
	{
		ImGui::BeginChild("##ReturnToFollowSmoother_TargetKeys", ImVec2(0.0f, 140.0f), true);
		for (size_t i = 0; i < targetKeyNames_.size();) {

			ImGui::PushID(static_cast<int>(i));

			const std::string& name = targetKeyNames_[i];
			const bool existsInEditor =
				(!availableKeys.empty() && std::binary_search(availableKeys.begin(), availableKeys.end(), name));
			if (existsInEditor) {
				ImGui::TextUnformatted(name.c_str());
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "%s (Missing)", name.c_str());
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove")) {
				targetKeyNames_.erase(targetKeyNames_.begin() + static_cast<std::ptrdiff_t>(i));
				ImGui::PopID();
				continue;
			}

			ImGui::PopID();
			++i;
		}
		ImGui::EndChild();
	}
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

	pitchRotateX_ = data.value("pitchRotateX_", pitchRotateX_);

	if (data.contains("targetKeyNames_") && data["targetKeyNames_"].is_array()) {

		targetKeyNames_.clear();
		for (const auto& keyName : data["targetKeyNames_"]) {
			if (keyName.is_string()) {
				targetKeyNames_.emplace_back(keyName.get<std::string>());
			}
		}
	}
}

void FollowCameraReturnToFollowSmoother::SaveJson() {

	Json data;

	data["durationSec_"] = duration_;
	data["easingType_"] = EnumAdapter<EasingType>::ToString(easingType_);
	data["pitchRotateX_"] = pitchRotateX_;

	for (const auto& keyName : targetKeyNames_) {

		data["targetKeyNames_"].emplace_back(keyName);
	}

	JsonAdapter::Save("Camera/Follow/returnToFollowSmoother.json", data);
}
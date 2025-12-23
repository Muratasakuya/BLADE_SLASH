#include "FollowCameraLookInputSmoother.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// inputDevice
#include <Game/Gameplay/Camera/FollowCamera/Input/Devices/FollowCameraKeyInput.h>
#include <Game/Gameplay/Camera/FollowCamera/Input/Devices/FollowCameraGamePadInput.h>

// imgui
#include <imgui.h>

//============================================================================
//	FollowCameraLookInputSmoother classMethods
//============================================================================

void FollowCameraLookInputSmoother::Init() {

	// 入力クラスを初期化
	SakuEngine::Input* input = SakuEngine::Input::GetInstance();
	inputMapper_ = std::make_unique<SakuEngine::InputMapper<FollowCameraInputAction>>();
	inputMapper_->AddDevice(std::make_unique<FollowCameraGamePadInput>(input));
	inputMapper_->AddDevice(std::make_unique<FollowCameraKeyInput>(input));

	// json適用
	ApplyJson();
}

void FollowCameraLookInputSmoother::Update(float deltaTime) {

	// 現在の入力タイプ
	InputType inputType = SakuEngine::Input::GetInstance()->GetType();
	// 入力値を取得する
	SakuEngine::Vector2 inputValue{};
	inputValue.x = inputMapper_->GetVector(FollowCameraInputAction::RotateX, inputType);
	inputValue.y = inputMapper_->GetVector(FollowCameraInputAction::RotateY, inputType);

	// 入力をフレーム補間で少しずつ補間する
	float t = std::clamp(inputLerpRate_ * deltaTime, 0.0f, 1.0f);
	smoothedInput_ = SakuEngine::Vector2::Lerp(smoothedInput_, inputValue, t);

	// 入力タイプに応じた感度
	bool isInputKeyboard = inputType == InputType::Keyboard;
	SakuEngine::Vector2 sensitivity = isInputKeyboard ? mouseSensitivity_ : padSensitivity_;

	// 1フレーム分の回転量
	float sensitivityDeltaTime = (isInputKeyboard ? 1.0f : deltaTime);
	// 横回転
	frameRotationDelta_.x = smoothedInput_.x * sensitivity.x * sensitivityDeltaTime;
	// 縦回転、マウスとゲームパッドで感度符号を変える
	frameRotationDelta_.y = (isInputKeyboard ? smoothedInput_.y : -smoothedInput_.y) * sensitivity.y * sensitivityDeltaTime;
}

void FollowCameraLookInputSmoother::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Input");

	// 現在の入力タイプ
	InputType inputType = SakuEngine::Input::GetInstance()->GetType();
	ImGui::Text("InputType: %s", SakuEngine::EnumAdapter<InputType>::ToString(inputType));

	// 入力値を取得する
	SakuEngine::Vector2 inputValue{};
	inputValue.x = inputMapper_->GetVector(FollowCameraInputAction::RotateX, inputType);
	inputValue.y = inputMapper_->GetVector(FollowCameraInputAction::RotateY, inputType);
	ImGui::Text("InputValue:         (%.2f / %.2f)", inputValue.x, inputValue.y);

	ImGui::Text("SmoothedInput:      (%.2f / %.2f)", smoothedInput_.x, smoothedInput_.y);
	ImGui::Text("FrameRotationDelta: (%.2f / %.2f)", frameRotationDelta_.x, frameRotationDelta_.y);

	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("inputLerpRate", &inputLerpRate_, 0.001f);
	ImGui::DragFloat2("padSensitivity", &padSensitivity_.x, 0.001f);
	ImGui::DragFloat2("mouseSensitivity", &mouseSensitivity_.x, 0.001f);
}

void FollowCameraLookInputSmoother::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/lookInputSmoother.json", data)) {
		return;
	}

	inputLerpRate_ = data.value("inputLerpRate_", 0.08f);
	padSensitivity_ = SakuEngine::Vector2::FromJson(data.value("padSensitivity_", Json()));
	mouseSensitivity_ = SakuEngine::Vector2::FromJson(data.value("mouseSensitivity_", Json()));
}

void FollowCameraLookInputSmoother::SaveJson() {

	Json data;

	data["inputLerpRate_"] = inputLerpRate_;
	data["padSensitivity_"] = padSensitivity_.ToJson();
	data["mouseSensitivity_"] = mouseSensitivity_.ToJson();

	SakuEngine::JsonAdapter::Save("Camera/Follow/lookInputSmoother.json", data);
}
#include "GameInputDeviceController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Window/WinApp.h>
#include <Engine/Input/Input.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Config.h>

//============================================================================
//	GameInputDeviceController classMethods
//============================================================================

void GameInputDeviceController::Init() {

	// 入力クラスを初期化
	SakuEngine::Input* input = SakuEngine::Input::GetInstance();
	gamepad_ = std::make_unique<GameInputGamePadInput>(input);
	keyboard_ = std::make_unique<GameInputKeyInput>(input);

	// 両方有効
	validInputTypes_[static_cast<uint32_t>(InputType::GamePad)] = true;
	validInputTypes_[static_cast<uint32_t>(InputType::Keyboard)] = true;

	// json適用
	ApplyJson();

	// マウスフラグ設定
	// リリース起動したとき、マウスを非表示、移動制御する
#if defined(_RELEASE)

	isDisplayMouse_ = false;
	isControlMoveMouse_ = true;
#else

	isDisplayMouse_ = true;
	isControlMoveMouse_ = false;
#endif
	// フラグの初期化
	preIsDisplayMouse_ = !isDisplayMouse_;
	preIsControlMoveMouse_ = !isControlMoveMouse_;
	UpdateMouseControl();
}

void GameInputDeviceController::Update() {

	// マウス制御の更新
	UpdateMouseControl();
	// 入力タイプの更新
	UpdateInputType();
}

void GameInputDeviceController::NonControlUpdate() {

	// クリップ解除
	SakuEngine::WinApp::ReleaseCursorClip();
	// マウス表示制御
	if (preIsDisplayMouse_ != isDisplayMouse_) {

		SakuEngine::WinApp::SetCursorVisible(isDisplayMouse_);
		preIsDisplayMouse_ = isDisplayMouse_;
	}
}

void GameInputDeviceController::ResultUpdateMouse(bool isDisplayFinished) {

	// Resultは常に範囲制限しない
	SakuEngine::WinApp::ReleaseCursorClip();

	// 直近デバイスを更新
	SakuEngine::Input* input = SakuEngine::Input::GetInstance();

	// 同フレームで両方来たらキーボード優先
	if (keyboard_->IsTriggered(GameInputAction::Use)) {

		inputType_ = InputType::Keyboard;
	} else if (gamepad_->IsTriggered(GameInputAction::Use)) {

		inputType_ = InputType::GamePad;
	}

	input->SetInputType(inputType_);
	// マウス表示制御
	bool visible = isDisplayFinished && (inputType_ == InputType::Keyboard);
	SakuEngine::WinApp::SetCursorVisible(visible);

	// フラグの更新
	isControlMoveMouse_ = false;
	isDisplayMouse_ = visible;

	// WinAppへ反映
	SakuEngine::WinApp::SetCursorVisible(visible);
	preIsDisplayMouse_ = isDisplayMouse_;
	preIsControlMoveMouse_ = isControlMoveMouse_;
}

void GameInputDeviceController::UpdateMouseControl() {

	// フラグが切り替わったタイミングで処理を行う
	// マウス表示制御
	if (preIsDisplayMouse_ != isDisplayMouse_) {
		if (isDisplayMouse_) {

			SakuEngine::WinApp::ReleaseCursorClip();
			SakuEngine::WinApp::SetCursorVisible(true);
		} else {

			SakuEngine::WinApp::SetCursorVisible(false);
		}
	}
	// マウス移動範囲制御
	if (preIsControlMoveMouse_ != isControlMoveMouse_) {
		if (isControlMoveMouse_) {

			SakuEngine::WinApp::ClipCursorToClientRect(mouseAreaSize_, mouseAreaPos_);
		} else {

			SakuEngine::WinApp::ReleaseCursorClip();
		}
	}
	// フラグの更新
	preIsDisplayMouse_ = isDisplayMouse_;
	preIsControlMoveMouse_ = isControlMoveMouse_;

	if (SakuEngine::Input::GetInstance()->PushKey(DIK_RETURN) &&
		SakuEngine::Input::GetInstance()->TriggerKey(DIK_F1)) {

		isControlMoveMouse_ = false;
	}
}

void GameInputDeviceController::UpdateInputType() {

	// ゲームパッドの入力をチェック
	if (gamepad_->IsTriggered(GameInputAction::Use)) {

		inputType_ = InputType::GamePad;
	} else if (keyboard_->IsTriggered(GameInputAction::Use)) {

		inputType_ = InputType::Keyboard;
	}
	// 有効な入力タイプかチェックする
	if (!validInputTypes_[static_cast<uint32_t>(InputType::GamePad)]) {

		// ゲームパッドが無効ならキーボードに切り替え
		inputType_ = InputType::Keyboard;
	} else if (!validInputTypes_[static_cast<uint32_t>(InputType::Keyboard)]) {

		// キーボードが無効ならゲームパッドに切り替え
		inputType_ = InputType::GamePad;
	}

	// 入力タイプを設定
	SakuEngine::Input::GetInstance()->SetInputType(inputType_);
}

void GameInputDeviceController::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::SeparatorText("Config");

	ImGui::Text("Input Type: %s", SakuEngine::EnumAdapter<InputType>::ToString(inputType_));

	ImGui::Checkbox("Enable GamePad", &validInputTypes_[static_cast<uint32_t>(InputType::GamePad)]);
	ImGui::Checkbox("Enable Keyboard", &validInputTypes_[static_cast<uint32_t>(InputType::Keyboard)]);
	// 両方falseにならないように制御
	if (!validInputTypes_[static_cast<uint32_t>(InputType::GamePad)] &&
		!validInputTypes_[static_cast<uint32_t>(InputType::Keyboard)]) {
		validInputTypes_[static_cast<uint32_t>(InputType::Keyboard)] = true;
	}

	ImGui::SeparatorText("Gamepad");

	if (ImGui::DragFloat("gamepadDeadZone", &gamepadDeadZone_, 1.0f)) {

		SakuEngine::Input::GetInstance()->SetDeadZone(gamepadDeadZone_);
	}

	ImGui::SeparatorText("Mouse");

	ImGui::Checkbox("Display Mouse", &isDisplayMouse_);
	ImGui::Checkbox("Control Move Mouse", &isControlMoveMouse_);
	ImGui::Text("Push Enter + F1 to release mouse control");
	{
		bool edit = false;
		edit |= ImGui::DragFloat2("Mouse Area Pos", &mouseAreaPos_.x, 1.0f);
		edit |= ImGui::DragFloat2("Mouse Area Size", &mouseAreaSize_.x, 1.0f);
		if (edit) {

			SakuEngine::WinApp::SetCursorClipRect(mouseAreaSize_, mouseAreaPos_);
		}
	}
}

void GameInputDeviceController::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("GameConfig/gameInput.json", data)) {
		return;
	}

	gamepadDeadZone_ = data.value("gamepadDeadZone_", 6400.0f);
	SakuEngine::Input::GetInstance()->SetDeadZone(gamepadDeadZone_);

	mouseAreaPos_ = SakuEngine::Vector2::FromJson(data.value("mouseAreaPos_", Json()));
	mouseAreaSize_ = SakuEngine::Vector2::FromJson(data.value("mouseAreaSize_", Json()));
}

void GameInputDeviceController::SaveJson() {

	Json data;

	data["gamepadDeadZone_"] = gamepadDeadZone_;
	data["mouseAreaPos_"] = mouseAreaPos_.ToJson();
	data["mouseAreaSize_"] = mouseAreaSize_.ToJson();

	SakuEngine::JsonAdapter::Save("GameConfig/gameInput.json", data);
}
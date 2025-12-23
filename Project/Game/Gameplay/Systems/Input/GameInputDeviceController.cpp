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

	// マウスフラグ設定
	isDisplayMouse_ = true;
	isControlMoveMouse_ = false;

	// json適用
	ApplyJson();
}

void GameInputDeviceController::Update() {

	// マウス制御の更新
	UpdateMouseControl();
	// 入力タイプの更新
	UpdateInputType();
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

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// ImGui上ではマウスを表示する
	ImGui::GetIO().MouseDrawCursor = !isDisplayMouse_;
	if (SakuEngine::Input::GetInstance()->TriggerKey(DIK_RETURN)) {

		isControlMoveMouse_ = false;
	}
#endif
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
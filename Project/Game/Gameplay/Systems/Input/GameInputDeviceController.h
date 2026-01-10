#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/InputStructures.h>
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/MathLib/Vector2.h>
#include <Game/Gameplay/Systems/Input/Devices/GameInputGamePadInput.h>
#include <Game/Gameplay/Systems/Input/Devices/GameInputKeyInput.h>

// c++
#include <memory>

//============================================================================
//	GameInputDeviceController class
//	入力デバイスの検知を行い入力タイプの更新を行うクラス
//============================================================================
class GameInputDeviceController :
	public SakuEngine::IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameInputDeviceController() :SakuEngine::IGameEditor("GameInputDevice") {}
	~GameInputDeviceController() = default;

	// 初期化
	void Init();

	// 更新
	void Update();
	// マウス制御を行わない更新、非表示のみ行う
	void NonControlUpdate();
	void ResultUpdateMouse(bool isDisplayFinished);

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// フラグの設定
	void SetIsDisplayMouse(bool isDisplay) { isDisplayMouse_ = isDisplay; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 有効な入力タイプ
	std::array<bool, static_cast<uint32_t>(InputType::Count)> validInputTypes_;

	// 現在の入力タイプ
	InputType inputType_;
	// デッドゾーン値
	float gamepadDeadZone_;

	// 入力クラス
	std::unique_ptr<GameInputGamePadInput> gamepad_; // ゲームパッド入力
	std::unique_ptr<GameInputKeyInput> keyboard_;    // キーボード入力

	// マウス制御
	bool isDisplayMouse_;     // マウス表示するか
	bool preIsDisplayMouse_;  // 前フレームのマウス表示フラグ
	bool isControlMoveMouse_;    // 移動制御するか
	bool preIsControlMoveMouse_; // 前フレームの移動制御フラグ
	SakuEngine::Vector2 mouseAreaPos_;  // マウス移動可能範囲位置
	SakuEngine::Vector2 mouseAreaSize_; // マウス移動可能範囲サイズ

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// マウス制御の更新
	void UpdateMouseControl();
	// 入力タイプの更新
	void UpdateInputType();
};
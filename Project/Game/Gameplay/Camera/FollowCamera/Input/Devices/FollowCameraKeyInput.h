#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Gameplay/Camera/FollowCamera/Input/Actions/FollowCameraInputAction.h>

//============================================================================
//	FollowCameraKeyInput class
//	キーボード入力
//============================================================================
class FollowCameraKeyInput :
	public SakuEngine::IInputDevice<FollowCameraInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraKeyInput(SakuEngine::Input* input) { input_ = input; }
	~FollowCameraKeyInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(FollowCameraInputAction axis)  const override;

	// 単入力
	bool IsPressed([[maybe_unused]] FollowCameraInputAction button) const override { return false; }
	bool IsTriggered([[maybe_unused]] FollowCameraInputAction button) const override { return false; }

	// 入力デバイスの種類を取得
	InputType GetInputType() const override { return InputType::Keyboard; }
};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Camera/Follow/Input/FollowCameraInputAction.h>

//============================================================================
//	FollowCameraGamePadInput class
//	ゲームパッド入力
//============================================================================
class FollowCameraGamePadInput :
	public SakuEngine::IInputDevice<FollowCameraInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraGamePadInput(SakuEngine::Input* input) { input_ = input; }
	~FollowCameraGamePadInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(FollowCameraInputAction axis)  const override;

	// 単入力
	bool IsPressed(FollowCameraInputAction button) const override;
	bool IsTriggered(FollowCameraInputAction button) const override;
};
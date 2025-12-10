#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Camera/Follow/Input/FollowCameraInputAction.h>

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
	bool IsPressed(FollowCameraInputAction button) const override;
	bool IsTriggered(FollowCameraInputAction button) const override;
};
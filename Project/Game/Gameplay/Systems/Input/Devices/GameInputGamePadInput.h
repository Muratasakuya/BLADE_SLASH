#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Gameplay/Systems/Input/Actions/GameInputAction.h>

//============================================================================
//	GameInputGamePadInput class
//	ゲームパッド入力
//============================================================================
class GameInputGamePadInput :
	public SakuEngine::IInputDevice<GameInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameInputGamePadInput(SakuEngine::Input* input) { input_ = input; }
	~GameInputGamePadInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(GameInputAction axis)  const override;

	// 単入力
	bool IsPressed([[maybe_unused]] GameInputAction button) const override { return false; }
	bool IsTriggered(GameInputAction button) const override;
};
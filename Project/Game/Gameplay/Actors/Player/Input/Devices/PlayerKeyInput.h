#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Gameplay/Actors/Player/Input/Actions/PlayerInputAction.h>

//============================================================================
//	PlayerKeyInput class
//	キーボード入力
//============================================================================
class PlayerKeyInput :
	public SakuEngine::IInputDevice<PlayerInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerKeyInput(SakuEngine::Input* input) { input_ = input; }
	~PlayerKeyInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(PlayerInputAction axis)  const override;

	// 単入力
	bool IsPressed(PlayerInputAction button) const override;
	bool IsTriggered(PlayerInputAction button) const override;

	// 入力デバイスの種類を取得
	InputType GetInputType() const override { return InputType::Keyboard; }
};
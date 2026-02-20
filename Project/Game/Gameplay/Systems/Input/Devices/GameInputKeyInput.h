#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Gameplay/Systems/Input/Actions/GameInputAction.h>

// windows
#include <windows.h>
// c++
#include <vector>

//============================================================================
//	GameInputKeyInput class
//	キーボード入力
//============================================================================
class GameInputKeyInput :
	public SakuEngine::IInputDevice<GameInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameInputKeyInput(SakuEngine::Input* input);
	~GameInputKeyInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(GameInputAction axis)  const override;

	// 単入力
	bool IsPressed([[maybe_unused]] GameInputAction button) const override { return false; }
	bool IsTriggered(GameInputAction button) const override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 入力判定を行うキーリスト
	std::vector<BYTE> keyList_;
};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/UI/Input/Actions/SelectUIInputAction.h>

//============================================================================
//	SelectUIKeyInput class
//	UI選択用キーボード入力
//============================================================================
class SelectUIKeyInput :
	public SakuEngine::IInputDevice<SelectUIInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SelectUIKeyInput(SakuEngine::Input* input) { input_ = input; }
	~SelectUIKeyInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(SelectUIInputAction axis)  const override;

	// 単入力
	bool IsPressed(SelectUIInputAction button) const override;
	bool IsTriggered(SelectUIInputAction button) const override;
};
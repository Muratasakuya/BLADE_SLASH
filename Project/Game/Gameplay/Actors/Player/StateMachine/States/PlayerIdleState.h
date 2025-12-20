#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/StateMachine/Interface/PlayerIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	PlayerIdleState class
//	何も入力していないときのアイドル状態、基本状態
//============================================================================
class PlayerIdleState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerIdleState();
	~PlayerIdleState() = default;

	void Enter() override;

	void Update() override;
	void UpdateAlways() override;

	void Exit() override;

	// imgui
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
};
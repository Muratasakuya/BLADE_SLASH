#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyFalterState class
//	怯み状態
//============================================================================
class BossEnemyFalterState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyFalterState() = default;
	~BossEnemyFalterState() = default;

	void Enter() override;

	void Update() override;

	void Exit() override;

	// imgui
	void ImGui() override;

	// json
	void ApplyJson([[maybe_unused]] const Json& data) override;
	void SaveJson([[maybe_unused]] Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// parameters
	float backStepSpeed_;    // 後ずさりするときの速度

	//--------- functions ----------------------------------------------------

};
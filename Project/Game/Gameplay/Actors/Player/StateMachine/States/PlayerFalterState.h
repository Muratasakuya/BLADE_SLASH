#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/StateMachine/Interface/PlayerIState.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	PlayerFalterState class
//	敵の攻撃を受けた時の怯み状態
//============================================================================
class PlayerFalterState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerFalterState() = default;
	~PlayerFalterState() = default;

	void Enter() override;

	void Update() override;

	void Exit() override;

	// imgui
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 怯みの移動補間時間
	SakuEngine::StateTimer falterTimer_;
	// 補間座標
	SakuEngine::Vector3 startPos_;
	SakuEngine::Vector3 targetPos_;
	// 移動距離
	float moveDistance_;

	// デルタタイムの停止時間
	float hitStopTime_;

	//--------- functions ----------------------------------------------------

};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/LerpValue.h>
#include <Game/Gameplay/Actors/Player/StateMachine/Interface/PlayerIState.h>

//============================================================================
//	PlayerParryWaitState class
//	プレイヤーがパリィ入力を行った時に発火するまで待機する状態
//============================================================================
class PlayerParryWaitState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerParryWaitState() = default;
	~PlayerParryWaitState() = default;

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

	// 敵との距離
	float enemyDistance_;

	// 座標補間時間
	SakuEngine::LerpValue<SakuEngine::Vector3> lerpPos_;

	//--------- functions ----------------------------------------------------

};
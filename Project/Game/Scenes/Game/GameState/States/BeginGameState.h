#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/Interface/IGameSceneState.h>

//============================================================================
//	BeginGameState class
//	ゲーム開始時の状態
//============================================================================
class BeginGameState :
	public IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BeginGameState(GameContext* context) :IGameSceneState(context){}
	~BeginGameState() = default;

	void Init(SakuEngine::SceneView* sceneView) override;

	void Update() override;
	void NonActiveUpdate() override;

	void ImGui() override;

	// 遷移開始時
	void Enter() override;
	// 遷移終了時
	void Exit() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 遷移後のプレイヤーの座標
	SakuEngine::Vector3 startPlayerPos_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
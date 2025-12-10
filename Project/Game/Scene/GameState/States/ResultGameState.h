#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/Interface/IGameSceneState.h>

//============================================================================
//	ResultGameState class
//	リザルト画面の状態
//============================================================================
class ResultGameState :
	public IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ResultGameState(GameContext* context) :IGameSceneState(context) {}
	~ResultGameState() = default;

	void Init(SakuEngine::SceneView* sceneView) override;

	void Update(SakuEngine::SceneManager* sceneManager) override;
	void NonActiveUpdate(SakuEngine::SceneManager* sceneManager) override;

	// 遷移開始時
	void Enter() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------



	//--------- functions ----------------------------------------------------

};
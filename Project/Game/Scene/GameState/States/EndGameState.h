#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/Interface/IGameSceneState.h>

//============================================================================
//	EndGameState class
//	敵、プレイヤーのどちらかが倒された後の状態
//============================================================================
class EndGameState :
	public IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EndGameState(GameContext* context) :IGameSceneState(context) {}
	~EndGameState() = default;

	void Init(SakuEngine::SceneView* sceneView) override;

	void Update(SakuEngine::SceneManager* sceneManager) override;
	void NonActiveUpdate(SakuEngine::SceneManager* sceneManager) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------



	//--------- functions ----------------------------------------------------

};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/Interface/IGameSceneState.h>

//============================================================================
//	PlayGameState class
//	ゲームプレイ中の状態
//============================================================================
class PlayGameState :
	public IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayGameState(GameContext* context) :IGameSceneState(context) {}
	~PlayGameState() = default;

	void Init(SakuEngine::SceneView* sceneView) override;

	void Update(SakuEngine::SceneManager* sceneManager) override;
	void NonActiveUpdate(SakuEngine::SceneManager* sceneManager) override;

	void ImGui() override;

	// 遷移開始時
	void Enter() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	//--------- functions ----------------------------------------------------

};
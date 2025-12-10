#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/Interface/IGameSceneState.h>

//============================================================================
//	StartGameState class
//	ゲーム開始状態
//============================================================================
class StartGameState :
	public IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	StartGameState(GameContext* context) :IGameSceneState(context) {}
	~StartGameState() = default;
	
	void Init(SakuEngine::SceneView* sceneView) override;

	void Update(SakuEngine::SceneManager* sceneManager) override;
	void NonActiveUpdate(SakuEngine::SceneManager* sceneManager) override;

	void Exit() override;

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 指定の範囲に入ったら次の状態に遷移させる
	std::unique_ptr<SakuEngine::Collider> nextStateEvent_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
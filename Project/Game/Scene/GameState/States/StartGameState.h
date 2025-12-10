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
	
	void Init(SceneView* sceneView) override;

	void Update(SceneManager* sceneManager) override;
	void NonActiveUpdate(SceneManager* sceneManager) override;

	void Exit() override;

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 指定の範囲に入ったら次の状態に遷移させる
	std::unique_ptr<Collider> nextStateEvent_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
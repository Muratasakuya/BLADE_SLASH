#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/GameContext.h>
#include <Game/Scene/GameState/GameSceneState.h>

//============================================================================
//	IGameSceneState class
//	ゲームシーンの状態のインターフェース
//============================================================================
class IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IGameSceneState(GameContext* context) :context_(context) {}
	virtual ~IGameSceneState() = default;

	// 初期化
	virtual void Init(SakuEngine::SceneView* sceneView) = 0;

	// 更新処理
	virtual void Update() = 0;
	virtual void NonActiveUpdate() {}

	// 遷移開始時
	virtual void Enter() {}
	// 遷移終了時
	virtual void Exit() {}

	// エディター
	virtual void ImGui() {}

	//--------- accessor -----------------------------------------------------

	bool IsRequestNext() const { return requestNext_; }
	void ClearRequestNext() { requestNext_ = false; }
	ResultSelect GetResultSelect() const { return context_->result->GetResultSelect(); }
	GameSceneState GetNextState() const { return nextState_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	GameContext* context_;

	// 次の状態への遷移
	bool requestNext_ = false;
	// 次の状態
	GameSceneState nextState_ = GameSceneState::Start;
};
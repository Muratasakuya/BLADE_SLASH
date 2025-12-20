#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Methods/IScene.h>

// scene
#include <Game/Scenes/Game/GameState/Interface/IGameSceneState.h>
#include <Game/Scenes/Transition/FadeTransition.h>

//============================================================================
//	GameScene class
//	ゲームシーン
//============================================================================
class GameScene :
	public SakuEngine::IScene,
	public SakuEngine::IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameScene() :IGameEditor("GameScene") {};
	~GameScene();

	void Init() override;

	void Update() override;
	void EndFrame() override;

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// sceneState
	GameSceneState currentState_;
	GameContext context_;
	std::array<std::unique_ptr<IGameSceneState>, static_cast<uint32_t>(GameSceneState::Count)> states_;

	// scene
	std::unique_ptr<CameraManager> cameraManager_;
	std::unique_ptr<GameLight> gameLight_;
	std::unique_ptr<FadeTransition> fadeTransition_;
	// collision
	std::unique_ptr<FieldBoundary> fieldBoundary_;

	// objects
	std::unique_ptr<Player> player_;
	std::unique_ptr<BossEnemy> bossEnemy_;
	std::unique_ptr<GameResultDisplay> result_;

	// sprites
	std::unique_ptr<FadeSprite> fadeSprite_;

	// effects
	std::unique_ptr<SakuEngine::EffectGroup> fieldEffect_;

	//--------- functions ----------------------------------------------------

	// init
	void InitStates();

	// helper
	void RequestNextState(GameSceneState next);
};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>

// scene
#include <Game/Scenes/Game/GameState/GameSceneState.h>
// cameras
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Game/Gameplay/Camera/EndGame/EndGameCamera.h>
#include <Game/Gameplay/Camera/ResultCamera/ClearResultCamera.h>
// front
class Player;
class BossEnemy;
namespace SakuEngine {
	class SceneView;
}

//============================================================================
//	CameraManager class
//	全てのカメラを管理する
//============================================================================
class CameraManager :
	public SakuEngine::IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CameraManager() :IGameEditor("CameraManager") {};
	~CameraManager() = default;

	// 全てのカメラを初期化
	void Init(SakuEngine::SceneView* sceneView);

	// シーンの状態に応じてカメラを更新
	void Update(GameSceneState sceneState);

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 追従先の設定
	void SetTarget(const Player* player, const BossEnemy* bossEnemy);

	FollowCamera* GetFollowCamera() const { return followCamera_.get(); }
	EndGameCamera* GetEndGameCamera() const { return endGameCamera_.get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	SakuEngine::SceneView* sceneView_;
	GameSceneState preSceneState_;

	// 追従カメラ
	std::unique_ptr<FollowCamera> followCamera_;
	// ゲーム終了時のカメラ
	std::unique_ptr<EndGameCamera> endGameCamera_;
	// リザルト画面のカメラ
	std::unique_ptr<ClearResultCamera> resultCamera_;

	//--------- functions ----------------------------------------------------

	// 現在の状態をチェックしてカメラを切り替える
	void CheckSceneState(GameSceneState sceneState);
};
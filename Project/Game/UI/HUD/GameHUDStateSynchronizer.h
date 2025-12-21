#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Game/UI/HUD/Controllers/Player/PlayerHUDController.h>
#include <Game/UI/HUD/Controllers/Enemy/Boss/BossEnemyHUDController.h>

//============================================================================
//	GameHUDStateSynchronizer class
//	Entityの状態とHUDの表示を同期させるクラス
//============================================================================
class GameHUDStateSynchronizer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameHUDStateSynchronizer() = default;
	~GameHUDStateSynchronizer() = default;

	// 初期化
	void Init(Player* player, BossEnemy* bossEnemy, const SakuEngine::BaseCamera* camera);

	// 更新
	void Update();

	// エディター
	void ImGui();

	// シーン状態変更の通知
	void NotifySceneStateChanged(GameSceneState prev, GameSceneState next);

	//--------- accessor -----------------------------------------------------

	const GameEventBus& GetEventBus() const { return bus_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	struct SceneUIRules {

		GameEventBus& bus_;
		GameHUDEvents::EntityId playerId_{}; // プレイヤーのエンティティID
		GameHUDEvents::EntityId bossId_{};   // ボスのエンティティID

		// ここに変数の説明が入る
		GameEventBus::Subscription<GameHUDEvents::SceneStateChangedEvent> subScene_;

		// 必要な値のセット
		SceneUIRules(GameEventBus& bus, GameHUDEvents::EntityId playerId, GameHUDEvents::EntityId bossId);

		// ここに関数の説明が入る
		void Install();
	};

	//--------- variables ----------------------------------------------------

	const Player* player_;
	const BossEnemy* boss_;
	const SakuEngine::BaseCamera* camera_;

	// ここに変数の説明が入る
	GameEventBus bus_;

	// プレイヤー
	std::unique_ptr<PlayerHUDController> playerHudController_;
	// ボス
	std::unique_ptr<BossEnemyHUDController> bossEnemyHudController_;

	// シーン状態に応じたUIルール
	std::unique_ptr<SceneUIRules> sceneRules_;

	//--------- functions ----------------------------------------------------

};
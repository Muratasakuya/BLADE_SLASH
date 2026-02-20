#include "GameHUDStateSynchronizer.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	GameHUDStateSynchronizer classMethods
//============================================================================

GameHUDStateSynchronizer::SceneUIRules::SceneUIRules(GameEventBus& bus,
	GameHUDEvents::EntityId playerId, GameHUDEvents::EntityId bossId)
	: bus_(bus), playerId_(playerId), bossId_(bossId) {
}

void GameHUDStateSynchronizer::SceneUIRules::Install() {

	// シーン状態変更イベントの発行
	subScene_ = bus_.Subscribe<GameHUDEvents::SceneStateChangedEvent>(
		[this](const GameHUDEvents::SceneStateChangedEvent& event) {
			switch (event.next) {
			case GameSceneState::Start:

				// 開始前はプレイヤーHUDのみ表示
				bus_.Publish(GameHUDEvents::VisibilityChangedEvent{ bossId_, false });
				bus_.Publish(GameHUDEvents::VisibilityChangedEvent{ playerId_, true });
				bus_.Publish(GameHUDEvents::TargetNavigationDisableEvent{ playerId_, true });
				bus_.Publish(GameHUDEvents::TargetNavigationInFrustumCheckEvent{ playerId_, false });
				break;
			case GameSceneState::BeginGame:

				// 登場演出中は矢印UI消す
				bus_.Publish(GameHUDEvents::TargetNavigationDisableEvent{ playerId_, true });
				bus_.Publish(GameHUDEvents::TargetNavigationInFrustumCheckEvent{ playerId_, false });
				break;
			case GameSceneState::PlayGame:

				// ゲーム中は矢印UI有効
				bus_.Publish(GameHUDEvents::TargetNavigationDisableEvent{ playerId_, false });
				bus_.Publish(GameHUDEvents::TargetNavigationInFrustumCheckEvent{ playerId_, true });
				// HUD表示を有効
				bus_.Publish(GameHUDEvents::VisibilityChangedEvent{ playerId_, true });
				break;
			case GameSceneState::EndGame:

				// 終了時は全部消す
				bus_.Publish(GameHUDEvents::VisibilityChangedEvent{ playerId_, false });
				bus_.Publish(GameHUDEvents::VisibilityChangedEvent{ bossId_, false });
				bus_.Publish(GameHUDEvents::TargetNavigationDisableEvent{ playerId_, true });
				break;
			}
		}
	);
}

void GameHUDStateSynchronizer::NotifySceneStateChanged(GameSceneState prev, GameSceneState next) {

	// Reusltに遷移した場合は何もしない
	if (next == GameSceneState::Result) {
		return;
	}

	// シーン状態変更イベントの発行
	bus_.Publish(GameHUDEvents::SceneStateChangedEvent{ prev, next });
}

void GameHUDStateSynchronizer::Init(Player* player,
	BossEnemy* bossEnemy, const SakuEngine::BaseCamera* camera) {

	// エンティティのセット
	player_ = player;
	boss_ = bossEnemy;
	camera_ = camera;

	// イベントバスの設定
	player->SetEventBus(&bus_);
	bossEnemy->SetEventBus(&bus_);

	// 各HUDコントローラーの初期化
	// プレイヤーHUDコントローラー
	playerHudController_ = std::make_unique<PlayerHUDController>(bus_, player, bossEnemy, camera);
	playerHudController_->Init();
	// ボスHUDコントローラー
	bossEnemyHudController_ = std::make_unique<BossEnemyHUDController>(bus_, bossEnemy, camera);
	bossEnemyHudController_->Init();

	// シーン状態に応じたUIルールの設定
	sceneRules_ = std::make_unique<SceneUIRules>(bus_,
		playerHudController_->GetPlayerId(), bossEnemyHudController_->GetBossId());
	sceneRules_->Install();

	// 初期状態の通知
	bus_.Publish(GameHUDEvents::SceneStateChangedEvent{ GameSceneState::Start, GameSceneState::Start });
}

void GameHUDStateSynchronizer::Update() {

	// 各HUDコントローラーの更新
	playerHudController_->Update();
	bossEnemyHudController_->Update();
}

void GameHUDStateSynchronizer::ImGui() {

	if (ImGui::BeginTabBar("PlayerHUDControllerTabBar")) {
		if (ImGui::BeginTabItem("Player")) {

			playerHudController_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("BossEnemy")) {

			bossEnemyHudController_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
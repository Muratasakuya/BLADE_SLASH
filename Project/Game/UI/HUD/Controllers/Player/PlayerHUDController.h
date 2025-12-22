#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Event/GameEventBus/GameEventBus.h>
#include <Game/Event/GameHUDEvents/GameHUDEvents.h>
#include <Game/UI/HUD/Interface/IGameHUDController.h>

// HUD
#include <Game/UI/HUD/Player/Share/PlayerShareHUD.h>
#include <Game/UI/HUD/Player/Stats/PlayerStatsHUD.h>
#include <Game/UI/HUD/Player/Operate/PlayerOperateHUD.h>
#include <Game/UI/HUD/Player/Stun/PlayerStunHUD.h>
#include <Game/UI/HUD/Player/TargetNavigation/TargetNavigation.h>

// front
class BossEnemy;

//============================================================================
//	PlayerHUDController class
//	プレイヤーHUDのコントローラー
//============================================================================
class PlayerHUDController :
	public IGameHUDController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerHUDController(GameEventBus& bus, const Player* player,
		const BossEnemy* bossEnemy, const SakuEngine::BaseCamera* camera);
	~PlayerHUDController() = default;

	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	GameHUDEvents::EntityId GetPlayerId() const { return playerId_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	GameEventBus& bus_;
	const Player* player_;
	const BossEnemy* bossEnemy_;
	const SakuEngine::BaseCamera* camera_;

	// 共有HUD部分
	std::unique_ptr<PlayerShareHUD> shareHud_;
	// ステータス表示
	std::unique_ptr<PlayerStatsHUD> statsHud_;
	// 操作方法表示
	std::unique_ptr<PlayerOperateHUD> operateHud_;
	// 敵がスタンしているときの表示
	std::unique_ptr<PlayerStunHUD> stunHud_;
	// 目標をナビゲートするUI
	std::unique_ptr<TargetNavigation> targetNavigation_;

	// ダメージイベント
	GameEventBus::Subscription<GameHUDEvents::DamageTakenEvent> subDamage_;
	// HUDの表示、非表示イベント
	GameEventBus::Subscription<GameHUDEvents::VisibilityChangedEvent> subVisible_;
	// 目標ナビゲーション無効化イベント
	GameEventBus::Subscription<GameHUDEvents::TargetNavigationDisableEvent> subNavDisable_;
	// 目標ナビゲーションのフラスタム内チェックイベント
	GameEventBus::Subscription<GameHUDEvents::TargetNavigationInFrustumCheckEvent> subNavFrustum_;
	// パリィ可能状態イベント
	GameEventBus::Subscription<GameHUDEvents::ParrySuggestEvent> subParrySuggest_;

	// ボスパリィ状態の終了を待つための変数
	std::optional<int32_t> exitParryBossStateInt_;
	bool isCanParryBoss_ = false;

	// プレイヤーのエンティティID
	GameHUDEvents::EntityId playerId_{};

	//--------- functions ----------------------------------------------------

	// パリィ時の入力示唆を更新
	void UpdateParrySuggest();
	// 目標ナビゲーションの更新
	void UpdateTargetNavigation();
};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Event/GameEventBus/GameEventBus.h>
#include <Game/Event/GameHUDEvents/GameHUDEvents.h>
#include <Game/UI/HUD/Interface/IGameHUDController.h>
#include <Game/UI/HUD/Enemy/Boss/BossEnemyHUD.h>

//============================================================================
//	BossEnemyHUDController class
//	ボスHUDのコントローラー
//============================================================================
class BossEnemyHUDController :
	public IGameHUDController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyHUDController(GameEventBus& bus,
		const BossEnemy* bossEnemy, const SakuEngine::BaseCamera* camera);
	~BossEnemyHUDController() = default;

	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	GameHUDEvents::EntityId GetBossId() const { return bossId_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	GameEventBus& bus_;
	const BossEnemy* bossEnemy_;
	const SakuEngine::BaseCamera* camera_;

	// HP、名前、ダメージ表示
	std::unique_ptr<BossEnemyHUD> bossHud_;

	// ダメージイベント
	GameEventBus::Subscription<GameHUDEvents::DamageTakenEvent> subDamage_;
	// HUDの表示、非表示イベント
	GameEventBus::Subscription<GameHUDEvents::VisibilityChangedEvent> subVisible_;

	// ボスのエンティティID
	GameHUDEvents::EntityId bossId_{};
};
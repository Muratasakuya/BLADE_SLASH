#include "BossEnemyHUDController.h"

//============================================================================
//	BossEnemyHUDController classMethods
//============================================================================

BossEnemyHUDController::BossEnemyHUDController(GameEventBus& bus,
	const BossEnemy* bossEnemy, const SakuEngine::BaseCamera* camera)
	: bus_(bus), bossEnemy_(bossEnemy), camera_(camera) {
}

void BossEnemyHUDController::Init() {

	// 各HUDの初期化
	// ボスHUD
	bossHud_ = std::make_unique<BossEnemyHUD>();
	bossHud_->Init();
	// 処理に必要な情報をセット
	bossHud_->SetBossEnemy(bossEnemy_);
	bossHud_->SetFollowCamera(camera_);
	// 最初は表示しない
	bossHud_->SetDisable();

	//============================================================================
	//	イベント登録
	//============================================================================

	bossId_ = GameHUDEvents::MakeEntityId(bossEnemy_);

	// ダメージイベントを登録
	subDamage_ = bus_.Subscribe<GameHUDEvents::DamageTakenEvent>(
		[this](const GameHUDEvents::DamageTakenEvent& event) {
			// 自分がダメージを受けた場合
			if (event.victim == bossId_) {

				// ダメージをHUDに反映
				bossHud_->SetDamage(event.damage);
			}
		}
	);
	// 表示状態変更イベントを登録
	subVisible_ = bus_.Subscribe<GameHUDEvents::VisibilityChangedEvent>(
		[this](const GameHUDEvents::VisibilityChangedEvent& event) {
			// 自分の表示状態変更の場合のみ処理
			if (event.target != bossId_) {
				return;
			}
			// 表示状態をHUDに反映
			if (event.visible) {

				bossHud_->SetValid();
			} else {

				bossHud_->SetDisable();
			}
		}
	);
}

void BossEnemyHUDController::Update() {

	// HUD更新
	bossHud_->Update();
}

void BossEnemyHUDController::ImGui() {
	
	// 各HUDのImGui表示
	// ボスHUD
	bossHud_->ImGui();
}
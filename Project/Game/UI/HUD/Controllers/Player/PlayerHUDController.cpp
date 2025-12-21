#include "PlayerHUDController.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	PlayerHUDController classMethods
//============================================================================

PlayerHUDController::PlayerHUDController(GameEventBus& bus, const Player* player,
	const BossEnemy* bossEnemy, const SakuEngine::BaseCamera* camera) :
	bus_(bus), player_(player), bossEnemy_(bossEnemy), camera_(camera) {
}

void PlayerHUDController::Init() {

	// 各HUDの初期化
	// プレイヤーHUD
	playerHud_ = std::make_unique<PlayerHUD>();
	playerHud_->Init();
	// 処理に必要な情報をセット
	playerHud_->SetPlayer(player_);
	playerHud_->SetCamera(camera_);
	playerHud_->SetBossEnemy(bossEnemy_);

	// スタンHUD
	stunHud_ = std::make_unique<PlayerStunHUD>();
	stunHud_->Init();

	// 目標ナビゲーションHUD
	targetNavigation_ = std::make_unique<TargetNavigation>();
	targetNavigation_->Init();
	// 処理に必要な情報をセット
	targetNavigation_->SetCamera(camera_);

	//============================================================================
	//	イベント登録
	//============================================================================

	playerId_ = GameHUDEvents::MakeEntityId(player_);

	// ダメージイベントを登録
	subDamage_ = bus_.Subscribe<GameHUDEvents::DamageTakenEvent>(
		[this](const GameHUDEvents::DamageTakenEvent& event) {
			// 自分がダメージを受けた場合
			if (event.victim == playerId_) {

				// ダメージをHUDに反映
				playerHud_->SetDamage(event.damage);
			}
		}
	);
	// HUDの表示、非表示イベントを登録
	subVisible_ = bus_.Subscribe<GameHUDEvents::VisibilityChangedEvent>(
		[this](const GameHUDEvents::VisibilityChangedEvent& event) {
			// 自分の表示状態変更の場合のみ処理
			if (event.target != playerId_) {
				return;
			}
			// 表示、非表示を切り替え
			if (event.visible) {

				playerHud_->SetValid();
			} else {

				playerHud_->SetDisable();
			}
		}
	);
	// 目標ナビゲーション無効化イベントを登録
	subNavDisable_ = bus_.Subscribe<GameHUDEvents::TargetNavigationDisableEvent>(
		[this](const GameHUDEvents::TargetNavigationDisableEvent& event) {
			// 自分の表示状態変更の場合のみ処理
			if (event.owner != playerId_) {
				return;
			}
			targetNavigation_->SetDisable(event.disable);
		}
	);
	// 目標ナビゲーションのフラスタム内チェックイベントを登録
	subNavFrustum_ = bus_.Subscribe<GameHUDEvents::TargetNavigationInFrustumCheckEvent>(
		[this](const GameHUDEvents::TargetNavigationInFrustumCheckEvent& event) {
			// 自分の表示状態変更の場合のみ処理
			if (event.owner != playerId_) {
				return;
			}
			targetNavigation_->SetInFrustumCheck(event.enable);
		}
	);
}

void PlayerHUDController::Update() {

	// HUD更新
	playerHud_->Update();

	// 敵がスタンしているときのHUD更新
	stunHud_->Update();

	// パリィ入力示唆
	UpdateParrySuggest();
	// ターゲットナビ
	UpdateTargetNavigation();
}

void PlayerHUDController::ImGui() {

	// 各HUDのImGui表示
	// プレイヤーHUD
	playerHud_->ImGui();

	// スタンHUD
	stunHud_->ImGui();

	// 目標ナビHUD
	targetNavigation_->ImGui();
}

void PlayerHUDController::UpdateParrySuggest() {

	// ボスのパリィ可能状態が終了したか確認
	if (exitParryBossStateInt_.has_value()) {
		if (exitParryBossStateInt_.value() != static_cast<int32_t>(bossEnemy_->GetCurrentState())) {

			exitParryBossStateInt_ = std::nullopt;
		}
	}

	// ボスのパリィ可能状態を取得
	const ParryParameter& parryParam = bossEnemy_->GetParryParam();
	// パリィが可能な状態になったら入力示唆を開始
	if (!exitParryBossStateInt_.has_value() && !isCanParryBoss_ && parryParam.canParry) {

		// パリィ可能状態に入った
		isCanParryBoss_ = true;
		playerHud_->StartInputSuggest();
		targetNavigation_->SetIsBlink(true);
	}

	// パリィ可能状態が終了したら入力示唆を終了
	if (isCanParryBoss_ && !parryParam.canParry) {

		isCanParryBoss_ = false;
		exitParryBossStateInt_ = static_cast<int32_t>(bossEnemy_->GetCurrentState());
		playerHud_->EndInputSuggest();
		targetNavigation_->SetIsBlink(false);
	}
}

void PlayerHUDController::UpdateTargetNavigation() {

	// 注視点、ピボットの座標をセットして更新
	targetNavigation_->SetTargetPos(bossEnemy_->GetTranslation());
	targetNavigation_->SetPivotPos(player_->GetTranslation());
	targetNavigation_->Update();
}
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
	// 共有HUD部分
	shareHud_ = std::make_unique<PlayerShareHUD>();
	shareHud_->Init();
	// 処理に必要な情報をセット
	shareHud_->SetPlayer(player_);
	// ステータス表示
	statsHud_ = std::make_unique<PlayerStatsHUD>();
	statsHud_->Init();
	// 処理に必要な情報をセット
	statsHud_->SetPlayer(player_);
	statsHud_->SetCamera(camera_);
	statsHud_->SetShareHUD(shareHud_.get());

	// 操作方法表示
	operateHud_ = std::make_unique<PlayerOperateHUD>();
	operateHud_->Init();
	// 処理に必要な情報をセット
	operateHud_->SetShareHUD(shareHud_.get());

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
				statsHud_->SetDamage(event.damage);
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
			statsHud_->SetIsDisplay(event.visible);
			operateHud_->SetIsDisplay(event.visible);
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
	// パリィ可能状態イベントを登録
	subParrySuggest_ = bus_.Subscribe<GameHUDEvents::ParrySuggestEvent>(
		[this](const GameHUDEvents::ParrySuggestEvent& event) {
			// 自分の表示状態変更の場合のみ処理
			if (event.owner != playerId_) {
				return;
			}
			// 入力示唆を開始、停止
			if (event.visible) {

				operateHud_->StartInputSuggest();
				targetNavigation_->SetIsBlink(true);
			} else {

				operateHud_->EndInputSuggest();
				targetNavigation_->SetIsBlink(false);
			}
		}
	);
}

void PlayerHUDController::Update() {

	// 共有HUD部分更新
	shareHud_->Update();
	// ステータス表示更新
	statsHud_->Update();
	// 操作方法表示更新
	operateHud_->Update();
	// 敵がスタンしているときのHUD更新
	stunHud_->Update();

	// パリィ入力示唆
	UpdateParrySuggest();
	// ターゲットナビ
	UpdateTargetNavigation();
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
		operateHud_->StartInputSuggest();
		targetNavigation_->SetIsBlink(true);
	}

	// パリィ可能状態が終了したら入力示唆を終了
	if (isCanParryBoss_ && !parryParam.canParry) {

		isCanParryBoss_ = false;
		exitParryBossStateInt_ = static_cast<int32_t>(bossEnemy_->GetCurrentState());
		operateHud_->EndInputSuggest();
		targetNavigation_->SetIsBlink(false);
	}
}

void PlayerHUDController::UpdateTargetNavigation() {

	// 注視点、ピボットの座標をセットして更新
	targetNavigation_->SetTargetPos(bossEnemy_->GetTranslation());
	targetNavigation_->SetPivotPos(player_->GetTranslation());
	targetNavigation_->Update();
}

void PlayerHUDController::ImGui() {

	// 各HUDのImGui表示
	if (ImGui::BeginTabBar("PlayerHUDControllerTabBar")) {
		if (ImGui::BeginTabItem("Share")) {

			// 共有HUD部分
			shareHud_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Stats")) {

			// ステータス表示HUD
			statsHud_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Operate")) {

			// 操作方法HUD
			operateHud_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Stun")) {

			// スタンHUD
			stunHud_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("TargetNav")) {

			// 目標ナビHUD
			targetNavigation_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
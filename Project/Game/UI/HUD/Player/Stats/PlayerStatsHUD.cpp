#include "PlayerStatsHUD.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/UI/HUD/Player/Share/PlayerShareHUD.h>

//============================================================================
//	PlayerStatsHUD classMethods
//============================================================================

void PlayerStatsHUD::Init() {

	// HP背景
	hpBackground_ = std::make_unique<SakuEngine::GameObject2D>();
	hpBackground_->Init("playerHPBackground", "hpBackground", "PlayerHUD");

	// HP残量
	hpBar_ = std::make_unique<GameHPBar>();
	hpBar_->Init("playerHPBar", "whiteAlphaGradation_1", "hpBar", "PlayerHUD");

	// スキル値
	skilBar_ = std::make_unique<GameHPBar>();
	skilBar_->Init("playerSkilBar", "whiteAlphaGradation_1", "destroyBar", "PlayerHUD");

	// 名前文字表示
	nameText_ = std::make_unique<SakuEngine::GameObject2D>();
	nameText_->Init("playerName", "playerName", "PlayerHUD");

	// ダメージ表示
	damageDisplay_ = std::make_unique<GameDisplayDamage>();
	damageDisplay_->Init("playerDamageNumber", "BossEnemyHUD", 4, 3);

	// スキルP閾値表示
	skillThreshold_ = std::make_unique<SakuEngine::GameObject2DArray>();
	skillThreshold_->Init();
	// スキルP閾値のスプライトを追加
	skillThreshold_->Add("barThresholdFrame", "skillThreshold", "PlayerHUD"); // 枠
	skillThreshold_->Add("barThreshold", "skillThreshold", "PlayerHUD");      // 中の色

	// json適用
	ApplyJson();
}

void PlayerStatsHUD::SetDamage(int damage) {

	// ダメージを設定
	damageDisplay_->SetDamage(damage);
}

void PlayerStatsHUD::SetIsDisplay(bool isDisplay) {

	// フラグに応じて設定するα値を設定する
	float alpha = isDisplay ? 1.0f : 0.0f;

	// α値を設定
	hpBackground_->SetAlpha(alpha);
	hpBar_->SetAlpha(alpha);
	skilBar_->SetAlpha(alpha);
	nameText_->SetAlpha(alpha);
	damageDisplay_->SetAlpha(alpha);
	skillThreshold_->SetAlpha(alpha);
}

void PlayerStatsHUD::Update() {

	// バーの更新
	UpdateBar();

	// ダメージ表記の更新
	damageDisplay_->Update(*player_, *camera_);

	// スキル閾値表示の更新
	UpdateSkillThreshold();
	skillThreshold_->Update();
}

void PlayerStatsHUD::UpdateBar() {

	// 頂点カラーアニメーション
	hpBarColorAnim_.Update(*hpBar_.get());

	const PlayerStats& stats = player_->GetStats();

	// HP残量を更新
	hpBar_->Update(stats.currentHP, stats.maxHP, true);
	// スキル値を更新
	skilBar_->Update(stats.currentSkillPoint, stats.maxSkillPoint, true);

	// スキル値の色を取得して設定
	skilBar_->SetColorRGB(shareHud_->GetCurrentSkillColor());
	skilBar_->SetUseVertexColor(shareHud_->IsActiveSkillColor());
	for (uint32_t i = 0; i < SakuEngine::kSpriteVertexPosNum; ++i) {

		// 頂点カラーを設定
		SakuEngine::SpriteVertexPos pos = static_cast<SakuEngine::SpriteVertexPos>(i);
		skilBar_->SetVertexColor(pos, shareHud_->GetCurrentSkillVertexColor(pos));
	}
}

void PlayerStatsHUD::UpdateSkillThreshold() {

	const PlayerStats& stats = player_->GetStats();

	// スキルP閾値表示の更新
	skillThreshold_->SetTranslation(skilBar_->GetThresholdPos(stats.skillCost));

	// スキル値の色を取得して設定
	skillThreshold_->SetColorRGB(shareHud_->GetCurrentSkillColor());
}

void PlayerStatsHUD::ImGui() {

	if (ImGui::BeginTabBar("PlayerStatsHUDTabBar")) {
		if (ImGui::BeginTabItem("Config")) {
			if (ImGui::Button("SaveJson")) {

				SaveJson();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Pos")) {
			if (hpBackgroundParameter_.ImGui("HPBackground")) {

				hpBackground_->SetTranslation(hpBackgroundParameter_.translation);
			}

			if (hpBarParameter_.ImGui("HPBar")) {

				hpBar_->SetTranslation(hpBarParameter_.translation);
			}

			if (skilBarParameter_.ImGui("SkilBar")) {

				skilBar_->SetTranslation(skilBarParameter_.translation);
			}

			if (nameTextParameter_.ImGui("NameText")) {

				nameText_->SetTranslation(nameTextParameter_.translation);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("DamageDisplay")) {

			damageDisplay_->ImGui();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("ColorAnim")) {

			if (ImGui::CollapsingHeader("HPBarColorAnim")) {

				hpBarColorAnim_.ImGui("HPBarColorAnim");
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PlayerStatsHUD::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Player/statsHUDParameters.json", data)) {
		return;
	}

	hpBackgroundParameter_.ApplyJson(data["hpBackground"]);
	GameCommon::SetInitParameter(*hpBackground_, hpBackgroundParameter_);
	hpBarParameter_.ApplyJson(data["hpBar"]);
	GameCommon::SetInitParameter(*hpBar_, hpBarParameter_);
	hpBarColorAnim_.FromJson(data.value("hpBarColorAnim", Json()));
	hpBarColorAnim_.Start();

	skilBarParameter_.ApplyJson(data["skilBar"]);
	GameCommon::SetInitParameter(*skilBar_, skilBarParameter_);

	nameTextParameter_.ApplyJson(data["nameText"]);
	GameCommon::SetInitParameter(*nameText_, nameTextParameter_);

	damageDisplay_->ApplyJson(data);
	skillThreshold_->FromJson(data.value("skillThreshold", Json()));
}

void PlayerStatsHUD::SaveJson() {

	Json data;

	hpBackgroundParameter_.SaveJson(data["hpBackground"]);
	hpBarParameter_.SaveJson(data["hpBar"]);
	hpBarColorAnim_.ToJson(data["hpBarColorAnim"]);
	skilBarParameter_.SaveJson(data["skilBar"]);
	nameTextParameter_.SaveJson(data["nameText"]);

	damageDisplay_->SaveJson(data);
	skillThreshold_->ToJson(data["skillThreshold"]);

	SakuEngine::JsonAdapter::Save("Player/statsHUDParameters.json", data);
}
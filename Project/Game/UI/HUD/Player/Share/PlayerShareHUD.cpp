#include "PlayerShareHUD.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerShareHUD classMethods
//============================================================================

void PlayerShareHUD::Init() {

	// スキル値頂点カラーアニメーション初期化
	skillVertexColorAnim_ = std::make_unique<SakuEngine::SpriteVertexColorAnimation>();
	enableSkillColor_ = SakuEngine::Vector3::AnyInit(1.0f);
	disableSkillColor_ = SakuEngine::Vector3::AnyInit(1.0f);

	// json適用
	ApplyJson();
}

const SakuEngine::Vector3& PlayerShareHUD::GetCurrentSkillColor() const {

	// フラグに応じて色を返す
	return isActiveSkillColor_ ? enableSkillColor_ : disableSkillColor_;
}

const SakuEngine::Color& PlayerShareHUD::GetCurrentSkillVertexColor(SakuEngine::SpriteVertexPos pos) const {

	return skillVertexColorAnim_->GetCurrentColor(pos);
}

void PlayerShareHUD::Update() {

	// スキル表示色更新
	UpdateSkillColor();
}

void PlayerShareHUD::UpdateSkillColor() {

	// スキル値頂点カラーアニメーション更新
	skillVertexColorAnim_->Update();

	const PlayerStats& stats = player_->GetStats();

	// スキルPに応じた色の更新
	isActiveSkillColor_ = stats.skillCost < stats.currentSkillPoint;
	if (isActiveSkillColor_) {

		// 頂点カラーアニメーション開始
		skillVertexColorAnim_->Start();
	} else {

		// 頂点カラーアニメーション停止
		skillVertexColorAnim_->Reset();
	}
}

void PlayerShareHUD::ImGui() {

	if (ImGui::BeginTabBar("PlayerShareHUD")) {
		if (ImGui::BeginTabItem("Config")) {
			if (ImGui::Button("SaveJson")) {

				SaveJson();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("SkillColor")) {

			ImGui::ColorEdit3("enableSkillColor", &enableSkillColor_.x);
			ImGui::ColorEdit3("disableSkillColor", &disableSkillColor_.x);

			ImGui::Separator();

			skillVertexColorAnim_->ImGui("SkillColorAnim");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PlayerShareHUD::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Player/shareHUDParameters.json", data)) {
		return;
	}

	skillVertexColorAnim_->FromJson(data.value("skillVertexColorAnim_", Json()));
	enableSkillColor_ = SakuEngine::Vector3::FromJson(data.value("enableSkillColor_", Json()));
	disableSkillColor_ = SakuEngine::Vector3::FromJson(data.value("disableSkillColor_", Json()));
}

void PlayerShareHUD::SaveJson() {

	Json data;

	skillVertexColorAnim_->ToJson(data["skillVertexColorAnim_"]);
	data["enableSkillColor_"] = enableSkillColor_.ToJson();
	data["disableSkillColor_"] = disableSkillColor_.ToJson();

	SakuEngine::JsonAdapter::Save("Player/shareHUDParameters.json", data);
}
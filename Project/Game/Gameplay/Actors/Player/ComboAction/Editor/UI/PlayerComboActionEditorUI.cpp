#include "PlayerComboActionEditorUI.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerActionNodeFactory.h>

// パネル
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Panels/PlayerActionNodePoolPanel.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Panels/PlayerComboListPanel.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Panels/PlayerActionNodeEditPanel.h>

//============================================================================
//	PlayerComboActionEditorUI Utility Methods
//============================================================================

PlayerComboActionEditorUI::PlayerComboActionEditorUI() {

	// 各パネル作成
	panels_.emplace_back(std::make_unique<PlayerActionNodePoolPanel>());
	panels_.emplace_back(std::make_unique<PlayerComboListPanel>());
	panels_.emplace_back(std::make_unique<PlayerActionNodeEditPanel>());

	// 別描画パネル作成
	timelinePanel_ = std::make_unique<PlayerComboTimelinePanel>();
	guardConditionPanel_ = std::make_unique<PlayerGuardConditionPanel>();
}

void PlayerComboActionEditorUI::Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	ImGui::SetWindowFontScale(0.64f);

	ImGuiTableFlags tableFlags =
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_BordersOuterV |
		ImGuiTableFlags_SizingStretchSame;

	if (ImGui::BeginTable("##layout", 3, tableFlags)) {

		// 各パネル描画
		for (const auto& panel : panels_) {

			ImGui::TableNextColumn();
			panel->Draw(model, select);
		}
		ImGui::EndTable();
	}
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Begin("Timeline", nullptr, ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

	ImGui::SetWindowFontScale(0.64f);

	if (ImGui::BeginTabBar("PlayerComboActionEditorUI")) {

		if (ImGui::BeginTabItem("Timeline")) {

			timelinePanel_->Draw(model, select);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Condition")) {

			guardConditionPanel_->Draw(model, select);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::SetWindowFontScale(1.0f);

	ImGui::End();
#endif
}
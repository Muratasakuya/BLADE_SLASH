#include "PlayerComboTimelinePanel.h"

//============================================================================
//	PlayerComboTimelinePanel classMethods
//============================================================================

void PlayerComboTimelinePanel::Draw(PlayerComboActionModel& model,
	PlayerComboActionEditorSelection& select) {

	ImGui::BeginChild("##timeline_panel", ImVec2(0.0f, 0.0f), true);

	ImGui::TextUnformatted("Timeline");
	ImGui::Separator();

	// タイムライン描画
	timeline_.Draw(model, select);

	ImGui::EndChild();
}
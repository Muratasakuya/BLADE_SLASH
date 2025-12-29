#include "PlayerActionNodeEditPanel.h"

//============================================================================
//	PlayerActionNodeEditPanel classMethods
//============================================================================

void PlayerActionNodeEditPanel::Draw(PlayerComboActionModel& model,
	PlayerComboActionEditorSelection& select) {

	ImGui::BeginChild("##node_edit_root", ImVec2(0, 410.0f), true,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImGui::TextUnformatted("Node Edit");
	ImGui::Separator();

	auto& nodes = model.ActionNodes();
	// 選択されていない場合
	if (select.selectedNodeIndex < 0 ||
		static_cast<size_t>(select.selectedNodeIndex) >= nodes.size()) {
		ImGui::TextUnformatted("No node selected.");
		ImGui::EndChild();
		return;
	}

	auto& node = nodes[static_cast<size_t>(select.selectedNodeIndex)];

	// ノード情報の表示
	ImGui::Text("id = %u", node.id);
	ImGui::SeparatorText(node.name.c_str());

	ImGui::BeginChild("##node_edit_scroll", ImVec2(0.0f, ImGui::GetContentRegionAvail().y),
		false, ImGuiWindowFlags_NoScrollbar);

	ImGui::PushItemWidth(imguiItemSize_);

	// ノード固有の編集UI
	node.implementation->ImGui();

	ImGui::PopItemWidth();

	ImGui::EndChild();
	ImGui::EndChild();
}
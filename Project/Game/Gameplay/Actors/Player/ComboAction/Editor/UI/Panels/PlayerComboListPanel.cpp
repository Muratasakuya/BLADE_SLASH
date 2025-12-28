#include "PlayerComboListPanel.h"

//============================================================================
//	PlayerComboListPanel classMethods
//============================================================================

void PlayerComboListPanel::Draw(PlayerComboActionModel& model,
	PlayerComboActionEditorSelection& select) {

	ImGui::BeginChild("##combo_list", ImVec2(0, 0), true);

	ImGui::TextUnformatted("Actions");
	ImGui::Separator();

	//============================================================================================================
	// 空のアクションを作成
	//============================================================================================================

	if (ImGui::Button("Create Action")) {

		model.CreateCombo("Action");
		select.selectedComboIndex = static_cast<int32_t>(model.Combos().size()) - 1;
		select.selectedStepIndex = -1;
	}

	ImGui::Spacing();
	ImGui::Separator();

	//============================================================================================================
	// アクション一覧
	//============================================================================================================

	ImGui::TextUnformatted("Action List");
	if (ImGui::BeginChild("##action_list_child", ImVec2(imguiItemSize_ + (imguiItemSize_ / 3.2f), imguiItemSize_ / 2.0f), true)) {

		auto& combos = model.Combos();
		for (int32_t i = 0; i < static_cast<int32_t>(combos.size()); ++i) {

			const auto& combo = combos[static_cast<size_t>(i)];
			bool selected = (select.selectedComboIndex == i);

			std::string line;
			line.reserve(128);
			line += "[";
			line += std::to_string(i);
			line += "] id=";
			line += std::to_string(combo.id);
			line += " ";
			line += combo.name;
			line += " steps=";
			line += std::to_string(combo.steps.size());

			if (ImGui::Selectable(line.c_str(), selected)) {

				select.selectedComboIndex = i;
				select.selectedStepIndex = -1;
			}
		}
		ImGui::EndChild();
	}

	//============================================================================================================
	// アクションの編集
	//============================================================================================================

	ImGui::TextUnformatted("Selected Action");

	ImGui::PushItemWidth(imguiItemSize_);

	auto& combos = model.Combos();
	if (select.selectedComboIndex >= 0 && select.selectedComboIndex < static_cast<int32_t>(combos.size())) {

		//============================================================================================================
		// コンボ情報の表示
		//============================================================================================================

		auto& combo = combos[static_cast<size_t>(select.selectedComboIndex)];

		ImGui::Text("id = %u", combo.id);

		// 名前編集
		PlayerComboPanelHelper::InputTextStdString("Name", &combo.name);

		ImGui::Spacing();

		//============================================================================================================
		// 複製、削除
		//============================================================================================================

		bool canMoveUp = (select.selectedComboIndex > 0);
		bool canMoveDown = (select.selectedComboIndex + 1 < static_cast<int>(combos.size()));

		if (ImGui::Button("Duplicate")) {

			model.DuplicateCombo(static_cast<size_t>(select.selectedComboIndex));
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {

			model.RemoveCombo(static_cast<size_t>(select.selectedComboIndex));
			select.selectedComboIndex = -1;
			select.selectedStepIndex = -1;
		}

		//============================================================================================================
		// ノード要素の入れ替え
		//============================================================================================================

		if (!canMoveUp) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Up")) {

			size_t a = static_cast<size_t>(select.selectedComboIndex);
			size_t b = static_cast<size_t>(select.selectedComboIndex - 1);
			if (model.SwapCombos(a, b)) {

				select.selectedComboIndex -= 1;
				select.selectedStepIndex = -1;
			}
		}
		if (!canMoveUp) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (!canMoveDown) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Down")) {

			size_t a = static_cast<size_t>(select.selectedComboIndex);
			size_t b = static_cast<size_t>(select.selectedComboIndex + 1);
			if (model.SwapCombos(a, b)) {

				select.selectedComboIndex += 1;
				select.selectedStepIndex = -1;
			}
		}
		if (!canMoveDown) {
			ImGui::EndDisabled();
		}
	} else {

		ImGui::TextUnformatted("No action selected.");
	}

	ImGui::PopItemWidth();
	ImGui::EndChild();
}
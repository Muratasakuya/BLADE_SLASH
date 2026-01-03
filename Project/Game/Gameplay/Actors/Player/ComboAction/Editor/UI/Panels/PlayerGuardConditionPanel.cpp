#include "PlayerGuardConditionPanel.h"

//============================================================================
//	PlayerGuardConditionPanel classMethods
//============================================================================

namespace {

	// 子ウィンドウの幅を半分に計算
	float CalcHalfChildWidth() {

		const float avail = ImGui::GetContentRegionAvail().x;
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		return (avail - spacing) * 0.5f;
	}
}

void PlayerGuardConditionPanel::Draw(PlayerComboActionModel& model,
	PlayerComboActionEditorSelection& select) {

	// コンボ未選択
	auto& combos = model.Combos();
	if (select.selectedComboIndex < 0 || combos.size() <= static_cast<size_t>(select.selectedComboIndex)) {

		ImGui::TextDisabled("UnSelected ComboAction");
		selectedStartConditionIndex_ = -1;
		lastComboIndex_ = -1;
		return;
	}

	// コンボ切り替えで選択リセット
	if (lastComboIndex_ != select.selectedComboIndex) {

		selectedStartConditionIndex_ = -1;
		lastComboIndex_ = select.selectedComboIndex;
	}

	auto& combo = combos[static_cast<size_t>(select.selectedComboIndex)];
	auto& startConditions = combo.startConditions;

	// 選択の範囲を調整
	if (selectedStartConditionIndex_ >= static_cast<int32_t>(startConditions.size())) {
		selectedStartConditionIndex_ = static_cast<int32_t>(startConditions.size()) - 1;
	}

	ImGui::TextUnformatted(std::format("Combo: {} (id:{})", combo.name, combo.id).c_str());
	ImGui::Separator();

	const float halfW = CalcHalfChildWidth();

	//================================================================================================================================
	// Left、追加、削除、リスト
	//================================================================================================================================

	ImGui::BeginChild("##StartComboCondition_Left", ImVec2(halfW, 0.0f), true);
	{
		ImGui::TextUnformatted("StartComboConditions");
		ImGui::Separator();

		// 追加UI
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Add");
		ImGui::SameLine();
		SakuEngine::EnumAdapter<PlayerGuardConditionType>::Combo("##NewStartConditionType", &newStartConditionType_);
		ImGui::SameLine();

		// 追加ボタン
		if (ImGui::Button("Add##StartCondition")) {

			model.AddStartComboCondition(static_cast<size_t>(select.selectedComboIndex), newStartConditionType_);
			selectedStartConditionIndex_ = static_cast<int32_t>(startConditions.size()) - 1;
		}

		// 選択中操作
		const bool hasSelection = (0 <= selectedStartConditionIndex_) &&
			(selectedStartConditionIndex_ < static_cast<int32_t>(startConditions.size()));

		ImGui::BeginDisabled(!hasSelection);
		{
			ImGui::SameLine();
			if (ImGui::Button("Remove##StartCondition")) {

				startConditions.erase(startConditions.begin() + static_cast<std::ptrdiff_t>(selectedStartConditionIndex_));
				if (startConditions.empty()) {

					selectedStartConditionIndex_ = -1;
				} else {

					selectedStartConditionIndex_ = (std::min)(selectedStartConditionIndex_,
						static_cast<int32_t>(startConditions.size()) - 1);
				}
			}

			if (ImGui::Button("Up")) {

				const int32_t i = selectedStartConditionIndex_;
				if (0 < i) {

					std::swap(startConditions[static_cast<size_t>(i)],
						startConditions[static_cast<size_t>(i - 1)]);
					selectedStartConditionIndex_ = i - 1;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Down")) {

				const int32_t i = selectedStartConditionIndex_;
				if (0 <= i && static_cast<size_t>(i + 1) < startConditions.size()) {

					std::swap(startConditions[static_cast<size_t>(i)],
						startConditions[static_cast<size_t>(i + 1)]);
					selectedStartConditionIndex_ = i + 1;
				}
			}
		}
		ImGui::EndDisabled();

		ImGui::Spacing();
		ImGui::Separator();

		// リスト
		ImVec2 listSize(-FLT_MIN, ImGui::GetContentRegionAvail().y);
		if (ImGui::BeginListBox("##StartConditionList", listSize)) {

			for (int32_t i = 0; i < static_cast<int32_t>(startConditions.size()); ++i) {

				const auto& cond = startConditions[static_cast<size_t>(i)];
				const bool selected = (i == selectedStartConditionIndex_);
				const char* typeName = SakuEngine::EnumAdapter<PlayerGuardConditionType>::ToString(cond.type);

				const std::string label = std::format("[{}] {}", i, typeName);
				if (ImGui::Selectable(label.c_str(), selected)) {

					selectedStartConditionIndex_ = i;
				}
			}
			ImGui::EndListBox();
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	//================================================================================================================================
	// 実装のImGui
	//================================================================================================================================

	ImGui::BeginChild("##StartComboCondition_Right", ImVec2(halfW, 0.0f), true);
	{
		ImGui::TextUnformatted("Selected Condition");
		ImGui::Separator();

		const bool hasSelection = (0 <= selectedStartConditionIndex_) &&
			(selectedStartConditionIndex_ < static_cast<int32_t>(startConditions.size()));

		if (!hasSelection) {

			ImGui::TextDisabled("Please Select LeftCondition");
		} else {

			auto& condition = startConditions[static_cast<size_t>(selectedStartConditionIndex_)];

			// タイプ変更
			PlayerGuardConditionType before = condition.type;
			// 作り直す
			if (SakuEngine::EnumAdapter<PlayerGuardConditionType>::Combo("Type", &condition.type)) {
				if (before != condition.type) {

					// 削除してから再作成
					model.RemoveStartComboCondition(static_cast<size_t>(select.selectedComboIndex), static_cast<size_t>(selectedStartConditionIndex_));
					model.AddStartComboCondition(static_cast<size_t>(select.selectedComboIndex), condition.type);
				}
			}

			ImGui::Separator();
			if (condition.implementation) {

				condition.implementation->ImGui();
			}
		}
	}
	ImGui::EndChild();
}
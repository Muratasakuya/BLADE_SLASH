#include "PlayerComboListPanel.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerComboActionJsonSerializer.h>

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

	//============================================================================================================
	// json 保存/読み込み
	//============================================================================================================

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("Json");

	// 保存/読み込み用のパス接頭辞
	static constexpr const char* kAllPrefix = "PlayerComboAction/";
	static constexpr const char* kNodePrefix = "PlayerComboAction/Nodes/";

	// 全て保存する
	if (ImGui::Button("Save")) {

		saveAllState_.showPopup = true;
		if (saveAllState_.input[0] == '\0') {

			std::snprintf(saveAllState_.input, SakuEngine::JsonSaveState::kBuffer, "ComboAction.json");
		}
	}

	ImGui::SameLine();

	// すべて読み込む
	if (ImGui::Button("Load")) {

		std::string rel;
		if (SakuEngine::ImGuiHelper::OpenJsonDialog(rel)) {
			if (PlayerComboActionJsonSerializer::LoadAllFromFile(rel, model)) {

				// 選択はリセット
				select.selectedComboIndex = -1;
				select.selectedStepIndex = -1;
				select.selectedNodeIndex = -1;
				select.selectedStepId = 0;
				status_ = "Loaded: " + rel;
			} else {

				status_ = "Load failed: " + rel;
			}
		}
	}

	// 保存モーダル
	{
		std::string outRelPath;
		const std::string baseLabel = std::string(SakuEngine::JsonAdapter::baseDirectoryFilePath_) + kAllPrefix;
		if (SakuEngine::ImGuiHelper::SaveJsonModal("Save ComboAction Json",
			baseLabel.c_str(), kAllPrefix, saveAllState_, outRelPath)) {

			PlayerComboActionJsonSerializer::SaveAllToFile(outRelPath, model);
			status_ = "Saved: " + outRelPath;
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("ActionNode Json");

	// 選択しているノードを保存
	const auto& nodes = model.ActionNodes();
	const bool canSaveNode = (select.selectedNodeIndex >= 0) &&
		(static_cast<size_t>(select.selectedNodeIndex) < nodes.size());

	// 保存できないなら無効化
	if (!canSaveNode) {

		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Save Node")) {

		saveNodeState_.showPopup = true;
		if (saveNodeState_.input[0] == '\0') {
			std::snprintf(saveNodeState_.input, SakuEngine::JsonSaveState::kBuffer, "ActionNode.json");
		}
	}
	if (!canSaveNode) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();

	// アクションノードの読み込み
	if (ImGui::Button("Load Node")) {

		std::string rel;
		if (SakuEngine::ImGuiHelper::OpenJsonDialog(rel)) {

			uint32_t newId = 0;
			if (PlayerComboActionJsonSerializer::LoadActionNodeFromFileToModel(rel, model, &newId)) {

				// 追加されたノードを選択にする
				int32_t newIndex = -1;
				for (int32_t i = 0; i < static_cast<int32_t>(model.ActionNodes().size()); ++i) {
					if (model.ActionNodes()[static_cast<size_t>(i)].id == newId) {
						newIndex = i;
						break;
					}
				}

				select.selectedNodeIndex = newIndex;
				status_ = "Imported Node: " + rel;
			} else {

				status_ = "Load Node failed: " + rel;
			}
		}
	}

	// ノード保存モーダル
	{
		std::string outRelPath;
		const std::string baseLabel = std::string(SakuEngine::JsonAdapter::baseDirectoryFilePath_) + kNodePrefix;
		if (SakuEngine::ImGuiHelper::SaveJsonModal("Save ActionNode Json",
			baseLabel.c_str(), kNodePrefix, saveNodeState_, outRelPath)) {

			if (canSaveNode) {

				const auto& node = model.ActionNodes()[static_cast<size_t>(select.selectedNodeIndex)];
				PlayerComboActionJsonSerializer::SaveActionNodeToFile(outRelPath, node);
				status_ = "Saved Node: " + outRelPath;
			} else {

				status_ = "Save Node failed: no node selected.";
			}
		}
	}

	// ステータス表示
	if (!status_.empty()) {

		ImGui::Spacing();
		ImGui::TextUnformatted(status_.c_str());
	}

	ImGui::EndChild();
}
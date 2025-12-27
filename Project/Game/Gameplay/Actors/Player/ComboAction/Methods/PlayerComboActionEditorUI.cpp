#include "PlayerComboActionEditorUI.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerActionNodeFactory.h>

//============================================================================
//	PlayerComboActionEditorUI Utility Methods
//============================================================================

namespace {

	// リサイズ用コールバック
	static int InputTextCallback_Resize(ImGuiInputTextCallbackData* data) {

		if (data->EventFlag != ImGuiInputTextFlags_CallbackResize) {
			return 0;
		}

		auto* str = static_cast<std::string*>(data->UserData);
		if (!str) {
			return 0;
		}

		// ImGui が必要とする長さ(data->BufTextLen)に合わせて string をリサイズ
		str->resize(static_cast<size_t>(data->BufTextLen));
		// resizeで再確保が起きる可能性があるので、Bufを更新
		data->Buf = str->data();
		return 0;
	}

	// std::stringをImGui::InputTextで編集するヘルパー
	inline bool InputTextStdString(const char* label, std::string* str, ImGuiInputTextFlags flags = 0) {

		if (!str) {
			return false;
		}

		flags |= ImGuiInputTextFlags_CallbackResize;
		// capacity()+1 は終端 '\0' 分
		return ImGui::InputText(label, str->data(), static_cast<int>(str->capacity() + 1),
			flags, InputTextCallback_Resize, str);
	}
}

//============================================================================
//	PlayerComboActionEditorUI classMethods
//============================================================================

void ActionNodePoolPanel::Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) {

	ImGui::BeginChild("##node_pool", ImVec2(0, 0), true);

	ImGui::TextUnformatted("Node Pool");
	ImGui::Separator();

	// 追加UI
	SakuEngine::EnumAdapter<PlayerActionNodeType>::Combo("ActionNodeType", &createType_);
	ImGui::SameLine();
	if (ImGui::Button("Add Node")) {

		model.AddActionNode(createType_);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("Nodes");
	ImGui::Spacing();

	// ノード一覧
	if (ImGui::BeginChild("##node_list", ImVec2(0, 260), true)) {

		auto& nodes = model.ActionNodes();
		for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {

			const auto& node = nodes[static_cast<std::size_t>(i)];
			const bool selected = (select.selectedNodeIndex == i);
			std::string line;
			line.reserve(128);
			line += "[";
			line += std::to_string(i);
			line += "] id=";
			line += std::to_string(node.id);
			line += "  ";
			line += node.name;
			line += "  (";
			line += SakuEngine::EnumAdapter<PlayerActionNodeType>::ToString(node.type);
			line += ")";
			if (ImGui::Selectable(line.c_str(), selected)) {

				select.selectedNodeIndex = i;
			}
		}
		ImGui::EndChild();
	}

	// 選択ノードの編集
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("Selected Node");

	auto& nodes = model.ActionNodes();
	if (select.selectedNodeIndex >= 0 && select.selectedNodeIndex < static_cast<int>(nodes.size())) {

		auto& node = nodes[static_cast<std::size_t>(select.selectedNodeIndex)];
		ImGui::Text("id = %u", node.id);

		// 名前編集
		InputTextStdString("Name", &node.name);

		// タイプ変更
		PlayerActionNodeType newType = node.type;
		if (SakuEngine::EnumAdapter<PlayerActionNodeType>::Combo("NodeType", &newType)) {

			node.type = newType;
			node.implementation = PlayerActionNodeFactory::CreateNode(node.type);
			if (node.name.empty()) {

				node.name = SakuEngine::EnumAdapter<PlayerActionNodeType>::ToString(node.type);
			}
		}

		ImGui::Spacing();

		// 操作ボタン
		bool canMoveUp = (select.selectedNodeIndex > 0);
		bool canMoveDown = (select.selectedNodeIndex + 1 < static_cast<int>(nodes.size()));

		if (ImGui::Button("Duplicate")) {

			model.DuplicateActionNode(static_cast<std::size_t>(select.selectedNodeIndex));
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {

			model.RemoveActionNode(static_cast<std::size_t>(select.selectedNodeIndex));
			select.selectedNodeIndex = -1;
		}

		if (!canMoveUp) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Up")) {

			size_t nodeAIndex = static_cast<std::size_t>(select.selectedNodeIndex);
			size_t nodeBIndex = static_cast<std::size_t>(select.selectedNodeIndex - 1);
			if (model.SwapActionNodes(nodeAIndex, nodeBIndex)) {

				select.selectedNodeIndex -= 1;
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

			size_t nodeAIndex = static_cast<std::size_t>(select.selectedNodeIndex);
			size_t nodeBIndex = static_cast<std::size_t>(select.selectedNodeIndex + 1);
			if (model.SwapActionNodes(nodeAIndex, nodeBIndex)) {

				select.selectedNodeIndex += 1;
			}
		}
		if (!canMoveDown) {

			ImGui::EndDisabled();
		}
	} else {

		ImGui::TextUnformatted("No node selected.");
	}
	ImGui::EndChild();
}

void ComboListPanel::Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) {

	ImGui::BeginChild("##combo_list", ImVec2(0, 0), true);

	ImGui::TextUnformatted("Actions");
	ImGui::Separator();

	// CreateAction()
	if (ImGui::Button("Create Action")) {

		model.CreateCombo("Action");
		select.selectedComboIndex = static_cast<int32_t>(model.Combos().size()) - 1;
		select.selectedStepIndex = -1;
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("Action List");
	ImGui::Spacing();

	// アクション一覧
	if (ImGui::BeginChild("##action_list_child", ImVec2(0, 260), true)) {

		auto& combos = model.Combos();
		for (int32_t i = 0; i < static_cast<int>(combos.size()); ++i) {

			const auto& combo = combos[static_cast<std::size_t>(i)];
			const bool selected = (select.selectedComboIndex == i);

			std::string line;
			line.reserve(128);
			line += "[";
			line += std::to_string(i);
			line += "] id=";
			line += std::to_string(combo.id);
			line += "  ";
			line += combo.name;
			line += "  steps=";
			line += std::to_string(combo.nodeAssetIds.size());

			if (ImGui::Selectable(line.c_str(), selected)) {

				select.selectedComboIndex = i;
				select.selectedStepIndex = -1;
			}
		}
		ImGui::EndChild();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("Selected Action");

	auto& combos = model.Combos();
	if (select.selectedComboIndex >= 0 && select.selectedComboIndex < static_cast<int32_t>(combos.size())) {

		auto& combo = combos[static_cast<size_t>(select.selectedComboIndex)];

		ImGui::Text("id = %u", combo.id);
		InputTextStdString("Name", &combo.name);

		ImGui::Spacing();

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

		if (!canMoveDown) { ImGui::BeginDisabled(); }
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
	ImGui::EndChild();
}

void ComboDetailPanel::Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) {

	ImGui::BeginChild("##combo_detail", ImVec2(0, 0), true);

	ImGui::TextUnformatted("Action Detail");
	ImGui::Separator();

	auto& combos = model.Combos();
	if (select.selectedComboIndex < 0 || select.selectedComboIndex >= static_cast<int>(combos.size())) {

		ImGui::TextUnformatted("Select an action from the middle panel.");
		ImGui::EndChild();
		return;
	}

	auto& combo = combos[static_cast<std::size_t>(select.selectedComboIndex)];

	// 選択中ノード
	std::uint32_t selectedNodeAssetId = 0;
	bool hasSelectedNode = false;
	if (select.selectedNodeIndex >= 0 && select.selectedNodeIndex < static_cast<int32_t>(model.ActionNodes().size())) {

		selectedNodeAssetId = model.ActionNodes()[static_cast<std::size_t>(select.selectedNodeIndex)].id;
		hasSelectedNode = true;
	}

	// 追加/挿入UI
	ImGui::Text("Editing: %s", combo.name.c_str());

	if (!hasSelectedNode) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Append Selected Node")) {

		model.AppendNodeToCombo(static_cast<size_t>(select.selectedComboIndex), selectedNodeAssetId);
	}
	if (!hasSelectedNode) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();

	bool canInsert = hasSelectedNode && (select.selectedStepIndex >= 0) &&
		(select.selectedStepIndex <= static_cast<int32_t>(combo.nodeAssetIds.size()));

	if (!canInsert) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Insert Before Step")) {

		size_t pos = static_cast<size_t>(select.selectedStepIndex);
		model.InsertNodeToCombo(static_cast<std::size_t>(select.selectedComboIndex), pos, selectedNodeAssetId);
	}
	ImGui::SameLine();
	if (ImGui::Button("Insert After Step")) {

		size_t pos = static_cast<size_t>(select.selectedStepIndex + 1);
		model.InsertNodeToCombo(static_cast<size_t>(select.selectedComboIndex), pos, selectedNodeAssetId);
	}
	if (!canInsert) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear Steps")) {

		model.ClearComboSteps(static_cast<size_t>(select.selectedComboIndex));
		select.selectedStepIndex = -1;
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("Steps");
	ImGui::Spacing();

	// ステップ一覧
	if (ImGui::BeginChild("##step_list", ImVec2(0, 260), true)) {

		const auto& ids = combo.nodeAssetIds;
		for (int i = 0; i < static_cast<int32_t>(ids.size()); ++i) {

			uint32_t id = ids[static_cast<std::size_t>(i)];
			const auto* asset = model.FindNodeAssetById(id);

			bool selected = (select.selectedStepIndex == i);

			std::string line;
			line.reserve(160);
			line += "[";
			line += std::to_string(i);
			line += "] ";

			if (asset) {

				line += "id=";
				line += std::to_string(asset->id);
				line += " ";
				line += asset->name;
				line += " (";
				line += SakuEngine::EnumAdapter<PlayerActionNodeType>::ToString(asset->type);
				line += ")";
			} else {

				line += "MissingNode id=";
				line += std::to_string(id);
			}

			if (!asset) {

				// Missingは色を変える
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
			}
			if (ImGui::Selectable(line.c_str(), selected)) {

				select.selectedStepIndex = i;
			}
			if (!asset) {
				ImGui::PopStyleColor();
			}
		}
		ImGui::EndChild();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("Selected Step");

	bool hasStep = (select.selectedStepIndex >= 0) && (select.selectedStepIndex < static_cast<int32_t>(combo.nodeAssetIds.size()));
	if (hasStep) {

		size_t stepIdx = static_cast<size_t>(select.selectedStepIndex);
		ImGui::Text("Step Index: %d", select.selectedStepIndex);
		ImGui::Text("NodeAssetId: %u", combo.nodeAssetIds[stepIdx]);

		bool canMoveUp = (select.selectedStepIndex > 0);
		bool canMoveDown = (select.selectedStepIndex + 1 < static_cast<int>(combo.nodeAssetIds.size()));

		if (ImGui::Button("Remove Step")) {

			model.RemoveComboStep(static_cast<size_t>(select.selectedComboIndex), stepIdx);
			select.selectedStepIndex = -1;
		}

		if (!canMoveUp) {
			ImGui::BeginDisabled();
		}
		ImGui::SameLine();
		if (ImGui::Button("Up")) {

			size_t a = stepIdx;
			size_t b = stepIdx - 1;
			if (model.SwapComboSteps(static_cast<size_t>(select.selectedComboIndex), a, b)) {
				select.selectedStepIndex -= 1;
			}
		}
		if (!canMoveUp) {
			ImGui::EndDisabled();
		}

		if (!canMoveDown) {
			ImGui::BeginDisabled();
		}
		ImGui::SameLine();
		if (ImGui::Button("Down")) {

			size_t a = stepIdx;
			size_t b = stepIdx + 1;
			if (model.SwapComboSteps(static_cast<size_t>(select.selectedComboIndex), a, b)) {

				select.selectedStepIndex += 1;
			}
		}
		if (!canMoveDown) {
			ImGui::EndDisabled();
		}
	} else {

		ImGui::TextUnformatted("No step selected. Select a step to edit / reorder / remove.");
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextUnformatted("Tips:");

	ImGui::BulletText("Select a node in the left panel, then append/insert it here.");
	ImGui::BulletText("Same node can be used multiple times: e.g. Attack0 -> Attack1 -> Attack0.");
	ImGui::BulletText("Deleting a node asset removes it from all actions.");

	ImGui::EndChild();
}

PlayerComboActionEditorUI::PlayerComboActionEditorUI() {

	// 各パネル作成
	nodePool_ = std::make_unique<ActionNodePoolPanel>();
	comboList_ = std::make_unique<ComboListPanel>();
	comboDetail_ = std::make_unique<ComboDetailPanel>();
}

void PlayerComboActionEditorUI::Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) {

	ImGuiTableFlags tableFlags =
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_BordersOuterV |
		ImGuiTableFlags_SizingStretchSame;

	if (ImGui::BeginTable("##layout", 3, tableFlags)) {

		ImGui::TableNextColumn();
		nodePool_->Draw(model, select);

		ImGui::TableNextColumn();
		comboList_->Draw(model, select);

		ImGui::TableNextColumn();
		comboDetail_->Draw(model, select);

		ImGui::EndTable();
	}
}
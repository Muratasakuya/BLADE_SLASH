#include "PlayerActionNodePoolPanel.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerActionNodeFactory.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/Timeline/Interface/IPlayerComboTimelineTrack.h>

//============================================================================
//	PlayerActionNodePoolPanel classMethods
//============================================================================

void PlayerActionNodePoolPanel::Draw(PlayerComboActionModel& model,
	PlayerComboActionEditorSelection& select) {

	ImGui::BeginChild("##node_pool", ImVec2(0, 0), true);

	ImGui::TextUnformatted("ActionNode Pool");
	ImGui::Separator();

	//============================================================================================================
	// タイプを選択してノードを追加
	//============================================================================================================

	ImGui::PushItemWidth(imguiItemSize_);
	SakuEngine::EnumAdapter<PlayerActionNodeType>::Combo("##Add", &createType_);
	ImGui::PopItemWidth();

	// ボタンで追加
	if (ImGui::Button("Add Node")) {

		model.AddActionNode(createType_);
	}
	ImGui::Spacing();
	ImGui::Separator();

	//============================================================================================================
	// ノード一覧
	//============================================================================================================

	ImGui::TextUnformatted("Nodes");
	if (ImGui::BeginChild("##node_list", ImVec2(imguiItemSize_ + (imguiItemSize_ / 4.0f), imguiItemSize_ / 2.0f), true)) {

		auto& nodes = model.ActionNodes();
		for (int32_t i = 0; i < static_cast<int32_t>(nodes.size()); ++i) {

			const auto& node = nodes[static_cast<size_t>(i)];
			bool selected = (select.selectedNodeIndex == i);
			std::string line;
			line.reserve(128);
			line += "[";
			line += std::to_string(i);
			line += "] id=";
			line += std::to_string(node.id);
			line += " ";
			line += node.name;
			if (ImGui::Selectable(line.c_str(), selected)) {

				select.selectedNodeIndex = i;
			}

			//============================================================================================================
			// Drag&Drop、Timelineに追加
			//============================================================================================================

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

				uint32_t assetId = node.id;
				ImGui::SetDragDropPayload(PlayerComboTimelinePayload::kActionNodeAssetId, &assetId, sizeof(assetId));
				ImGui::Text("%s", node.name.c_str());
				ImGui::EndDragDropSource();
			}
		}
		ImGui::EndChild();
	}

	//============================================================================================================
	// 選択ノードの編集
	//============================================================================================================

	ImGui::TextUnformatted("Selected Node");

	ImGui::PushItemWidth(imguiItemSize_);

	auto& nodes = model.ActionNodes();
	if (select.selectedNodeIndex >= 0 && select.selectedNodeIndex < static_cast<int32_t>(nodes.size())) {

		//============================================================================================================
		// ノード情報の表示
		//============================================================================================================

		auto& node = nodes[static_cast<size_t>(select.selectedNodeIndex)];

		ImGui::Text("id = %u", node.id);

		// 名前編集
		PlayerComboPanelHelper::InputTextStdString("Name", &node.name);

		// タイプ変更
		PlayerActionNodeType newType = node.type;
		if (SakuEngine::EnumAdapter<PlayerActionNodeType>::Combo("##Current", &newType)) {

			node.type = newType;
			node.implementation = PlayerActionNodeFactory::CreateNode(node.type);
			if (node.name.empty()) {

				node.name = SakuEngine::EnumAdapter<PlayerActionNodeType>::ToString(node.type);
			}
		}

		ImGui::Spacing();

		// 操作ボタン
		bool canMoveUp = (select.selectedNodeIndex > 0);
		bool canMoveDown = (select.selectedNodeIndex + 1 < static_cast<int32_t>(nodes.size()));

		//============================================================================================================
		// 複製、削除
		//============================================================================================================

		if (ImGui::Button("Duplicate")) {

			model.DuplicateActionNode(static_cast<size_t>(select.selectedNodeIndex));
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {

			model.RemoveActionNode(static_cast<size_t>(select.selectedNodeIndex));
			select.selectedNodeIndex = -1;
		}

		//============================================================================================================
		// ノード要素の入れ替え
		//============================================================================================================

		if (!canMoveUp) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Up")) {

			size_t nodeAIndex = static_cast<size_t>(select.selectedNodeIndex);
			size_t nodeBIndex = static_cast<size_t>(select.selectedNodeIndex - 1);
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

			size_t nodeAIndex = static_cast<size_t>(select.selectedNodeIndex);
			size_t nodeBIndex = static_cast<size_t>(select.selectedNodeIndex + 1);
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

	ImGui::PopItemWidth();
	ImGui::EndChild();
}
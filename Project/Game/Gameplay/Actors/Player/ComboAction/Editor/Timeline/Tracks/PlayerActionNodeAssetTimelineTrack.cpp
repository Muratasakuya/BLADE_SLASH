#include "PlayerActionNodeAssetTimelineTrack.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>

//============================================================================
//	PlayerActionNodeAssetTimelineTrack classMethods
//============================================================================

namespace {

	// 入力猶予がノード終了とほぼ一致しているか
	bool IsInputGraceLinked(const PlayerComboActionModel::ComboStep& step) {

		float endTime = step.startTime + step.duration;
		return std::fabs(step.inputGraceStartTime - endTime) <= Config::kEpsilon;
	}
}

void PlayerActionNodeAssetTimelineTrack::DrawTrack(
	PlayerComboTimelineDrawContext& context, float trackTopY) {

	auto& combo = context.model.Combos()[context.comboIndex];
	auto& steps = combo.steps;

	// トラック背景
	ImU32 bg = IM_COL32(30, 30, 30, 255);
	ImVec2 a = { context.contentScreenPos.x, trackTopY };
	ImVec2 b = { context.contentScreenPos.x + context.contentScreenSize.x, trackTopY + context.view.trackHeight };
	context.drawList->AddRectFilled(a, b, bg);

	//===================================================================================================================
	// 空クリックで作成ポップアップ
	//===================================================================================================================
	{
		ImGui::PushID("ActionNodeTrackEmpty");
		ImVec2 rowPos = { context.contentScreenPos.x, trackTopY };
		ImGui::SetCursorScreenPos(rowPos);
		ImGui::InvisibleButton("##empty_row_btn", ImVec2(context.contentScreenSize.x, context.view.trackHeight));

		//===================================================================================================================
		// Drag&Dropで追加
		//===================================================================================================================

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PlayerComboTimelinePayload::kActionNodeAssetId)) {
				if (payload->DataSize == sizeof(uint32_t)) {

					// ドロップされたアセットID
					uint32_t assetId = *static_cast<const uint32_t*>(payload->Data);

					// ドロップ位置の時間
					float localX = (ImGui::GetIO().MousePos.x - context.contentScreenPos.x) + context.scroll.x - context.view.contentPadding.x;
					float t = (std::max)(0.0f, PlayerComboTimelineHelper::LocalXToTime(context.view, localX));
					t = PlayerComboTimelineHelper::SnapTime(t);

					// ステップを追加
					uint32_t newStepId = 0;
					context.model.AddStepAtTime(context.comboIndex, assetId, t, &newStepId);
					context.model.SortStepsByStartTime(context.comboIndex);

					// 選択追従
					int32_t newIndex = context.model.FindStepIndexById(context.comboIndex, newStepId);
					context.select.selectedStepId = newStepId;
					context.select.selectedStepIndex = newIndex;
				}
			}
			ImGui::EndDragDropTarget();
		}

		//===================================================================================================================
		// 左クリックでノードを作成
		//===================================================================================================================

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {

			// クリック位置の時間を保存
			float localX = (ImGui::GetIO().MousePos.x - context.contentScreenPos.x) + context.scroll.x - context.view.contentPadding.x;
			float t = (std::max)(0.0f, PlayerComboTimelineHelper::LocalXToTime(context.view, localX));
			createTime_ = PlayerComboTimelineHelper::SnapTime(t);
			ImGui::OpenPopup("##CreateActionNodePopup");
		}

		//===================================================================================================================
		// 左クリックでノード作成ポップアップ
		//===================================================================================================================

		if (ImGui::BeginPopup("##CreateActionNodePopup")) {

			ImGui::TextUnformatted("Create ActionNode");
			SakuEngine::EnumAdapter<PlayerActionNodeType>::Combo("Type", &createType_);
			if (ImGui::Button("Create")) {

				// ノードを作成する
				uint32_t assetId = context.model.AddActionNode(createType_);

				// クリック時間にステップを追加する
				uint32_t newStepId = 0;
				context.model.AddStepAtTime(context.comboIndex, assetId, createTime_, &newStepId);
				context.model.SortStepsByStartTime(context.comboIndex);

				// 選択追従
				int32_t newIndex = context.model.FindStepIndexById(context.comboIndex, newStepId);
				context.select.selectedStepId = newStepId;
				context.select.selectedStepIndex = newIndex;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {

				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

	//===================================================================================================================
	// クリップ表示
	//===================================================================================================================

	for (int32_t i = 0; i < static_cast<int32_t>(steps.size()); ++i) {

		auto& step = steps[static_cast<size_t>(i)];
		auto* asset = context.model.FindNodeAssetById(step.nodeAssetId);

		float x0 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, step.startTime) - context.scroll.x;
		float x1 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, step.startTime + step.duration) - context.scroll.x;

		float y0 = trackTopY + 6.0f;
		float y1 = trackTopY + context.view.trackHeight - 6.0f;

		// 画面外は省略
		if (x1 < 0.0f || context.contentScreenSize.x < x0) {
			continue;
		}

		ImVec2 p0 = { context.contentScreenPos.x + x0, y0 };
		ImVec2 p1 = { context.contentScreenPos.x + x1, y1 };

		// 選択状態
		bool isSelected = (context.select.selectedStepId != 0 && context.select.selectedStepId == step.stepId);
		ImU32 color = isSelected ? IM_COL32(90, 150, 240, 255) : IM_COL32(80, 110, 150, 255);

		SakuEngine::ImGuiHelper::AddRectFilledRound(context.drawList, p0, p1, color, 6.0f);
		context.drawList->AddRect(p0, p1, IM_COL32(20, 20, 20, 255), 6.0f);

		// ラベル
		std::string label = asset != nullptr ? asset->name : "MissingNode";
		context.drawList->AddText(ImVec2(p0.x + 6.0f, p0.y + 6.0f), IM_COL32(255, 255, 255, 255), label.c_str());

		// クリック、ドラッグ
		ImGui::PushID(step.stepId);

		ImVec2 btnPos = { p0.x, p0.y };
		ImGui::SetCursorScreenPos(btnPos);
		ImGui::InvisibleButton("##clip_btn", ImVec2(p1.x - p0.x, p1.y - p0.y));

		//===================================================================================================================
		// Drag&Dropで差し替え
		//===================================================================================================================

		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PlayerComboTimelinePayload::kActionNodeAssetId)) {
				if (payload->DataSize == sizeof(uint32_t)) {

					// ドロップされたアセットID
					uint32_t assetId = *static_cast<const uint32_t*>(payload->Data);
					step.nodeAssetId = assetId;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// 選択
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {

			context.select.selectedStepId = step.stepId;
			context.select.selectedStepIndex = i;
		}

		//===================================================================================================================
		// 右クリック、削除
		//===================================================================================================================

		if (ImGui::BeginPopupContextItem("##clip_context")) {

			if (ImGui::MenuItem("Remove")) {

				// indexは変わる可能性があるので再検索
				int32_t index = context.model.FindStepIndexById(context.comboIndex, step.stepId);
				if (0 <= index) {
					context.model.RemoveComboStep(context.comboIndex, static_cast<size_t>(index));
				}
				context.select.selectedStepId = 0;
				context.select.selectedStepIndex = -1;
			}
			ImGui::EndPopup();
		}

		//===================================================================================================================
		// ドラッグ移動
		//===================================================================================================================

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			// 入力猶予がノード終了にリンクしている場合は、ノード移動に追従させる
			bool isGraceLinked = IsInputGraceLinked(step);
			float oldEnd = step.startTime + step.duration;

			// 移動量計算
			float deltaX = ImGui::GetIO().MouseDelta.x;
			float deltaT = deltaX / context.view.pixelsPerSecond;

			// 開始時間をマウス移動分ずらす
			float newStart = step.startTime + deltaT;
			newStart = (std::max)(0.0f, newStart);
			newStart = PlayerComboTimelineHelper::SnapTime(newStart);

			// 更新
			step.startTime = newStart;
			// 入力猶予も追従するか
			if (isGraceLinked) {

				float newEnd = step.startTime + step.duration;
				step.inputGraceStartTime += (newEnd - oldEnd);
				step.inputGraceStartTime = (std::max)(0.0f, PlayerComboTimelineHelper::SnapTime(step.inputGraceStartTime));
			}
			isMovedThisFrame_ = true;
		}

		//===================================================================================================================
		// 離したらソートさせる処理
		//===================================================================================================================

		if (isMovedThisFrame_ && ImGui::IsItemDeactivatedAfterEdit()) {

			uint32_t keepId = step.stepId;
			context.model.SortStepsByStartTime(context.comboIndex);

			// 選択追従
			int32_t newIndex = context.model.FindStepIndexById(context.comboIndex, keepId);
			context.select.selectedStepId = keepId;
			context.select.selectedStepIndex = newIndex;
			isMovedThisFrame_ = false;
		}

		// リサイズ
		float handleW = 10.0f;
		ImVec2 handle0 = { p1.x - handleW, p0.y };
		ImVec2 handle1 = { p1.x, p1.y };

		SakuEngine::ImGuiHelper::AddRectFilledRound(context.drawList, handle0, handle1, IM_COL32(220, 220, 220, 140), 4.0f);

		ImGui::SetCursorScreenPos(handle0);
		ImGui::InvisibleButton("##resize_handle", ImVec2(handleW, p1.y - p0.y));
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			bool isGraceLinked = IsInputGraceLinked(step);
			float oldEnd = step.startTime + step.duration;

			// 移動量計算
			float deltaX = ImGui::GetIO().MouseDelta.x;
			float deltaT = deltaX / context.view.pixelsPerSecond;

			// 処理時間をマウス移動分ずらす
			float newDuration = step.duration + deltaT;
			newDuration = (std::max)(context.view.gridStep, newDuration);
			newDuration = PlayerComboTimelineHelper::SnapTime(newDuration);
			step.duration = newDuration;

			// 入力猶予も追従するか
			if (isGraceLinked) {

				float newEnd = step.startTime + step.duration;
				step.inputGraceStartTime += (newEnd - oldEnd);
				step.inputGraceStartTime = (std::max)(0.0f, PlayerComboTimelineHelper::SnapTime(step.inputGraceStartTime));
			}
		}
		ImGui::PopID();
	}
}
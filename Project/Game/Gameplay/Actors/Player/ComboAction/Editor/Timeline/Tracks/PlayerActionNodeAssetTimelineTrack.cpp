#include "PlayerActionNodeAssetTimelineTrack.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

// imgui
#include <imgui_internal.h>

//============================================================================
//	PlayerActionNodeAssetTimelineTrack classMethods
//============================================================================

namespace {

	static void DrawStepInputSettingUI(PlayerComboActionModel::StepInputSetting& input) {

		ImGui::SeparatorText("Keyboard");
		ImGui::Checkbox("UseKeyboard", &input.isUseKeyboard);

		if (input.isUseKeyboard) {

			SakuEngine::EnumAdapter<KeyDIKCode>::Combo("Key", &input.keyDIKCode);
		}

		ImGui::SeparatorText("Mouse");
		ImGui::Checkbox("UseMouse", &input.isUseMouse);

		if (input.isUseMouse) {

			SakuEngine::EnumAdapter<MouseButton>::Combo("MouseButton", &input.mouseButton);
		}

		ImGui::SeparatorText("GamePad");
		ImGui::Checkbox("UseGamePad", &input.isUseGamePad);

		if (input.isUseGamePad) {

			SakuEngine::EnumAdapter<GamePadButtons>::Combo("PadButton", &input.gamePadButton);
		}
	}

	// マウスのX移動量を取得
	float GetMouseDeltaX() {

		auto* input = SakuEngine::Input::GetInstance();
		if (input) {
			return input->GetMouseMoveValue().x;
		}
		return ImGui::GetIO().MouseDelta.x;
	}

	// 省略テキストを作成
	std::string MakeEllipsisText(const std::string& text, float maxWidth) {

		const char* ellipsis = "...";
		float ellW = ImGui::CalcTextSize(ellipsis).x;

		if (maxWidth <= 0.0f) {
			return std::string();
		}

		// そのまま入る
		if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth) {
			return text;
		}

		// 省略記号すら入らない
		if (maxWidth <= ellW) {
			return std::string(ellipsis);
		}

		// 二分探索で入る文字数を探す
		int lo = 0;
		int hi = static_cast<int>(text.size());
		while (lo < hi) {

			int mid = (lo + hi) / 2;
			std::string sub = text.substr(0, static_cast<size_t>(mid));
			float w = ImGui::CalcTextSize(sub.c_str()).x;

			if (w + ellW <= maxWidth) {
				lo = mid + 1;
			} else {
				hi = mid;
			}
		}
		int count = (std::max)(0, lo - 1);
		return text.substr(0, static_cast<size_t>(count)) + ellipsis;
	}

	// マウスがクリップ上か判定（ActionNodeトラック用）
	bool IsMouseOverAnyClip(const PlayerComboTimelineDrawContext& context,
		const std::vector<PlayerComboActionModel::ComboStep>& steps,
		float trackTopY) {

		ImVec2 m = ImGui::GetIO().MousePos;

		for (const auto& step : steps) {

			auto* asset = context.model.FindNodeAssetById(step.nodeAssetId);

			float x0 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, step.startTime) - context.scroll.x;
			// 実装がある場合は実装の合計時間を優先
			float duration = step.duration;
			if (asset && asset->implementation) {

				duration = (std::max)(0.0f, asset->implementation->GetTotalTime());
			}
			float x1 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(
				context.view, step.startTime + duration) - context.scroll.x;

			float y0 = trackTopY + 6.0f;
			float y1 = trackTopY + context.view.trackHeight - 6.0f;

			// 最低幅（掴みやすさ）
			if ((x1 - x0) < 18.0f) {
				x1 = x0 + 18.0f;
			}

			ImVec2 p0 = { context.contentScreenPos.x + x0, y0 };
			ImVec2 p1 = { context.contentScreenPos.x + x1, y1 };

			if (p0.x <= m.x && m.x <= p1.x && p0.y <= m.y && m.y <= p1.y) {
				return true;
			}
		}

		return false;
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

	// グリッド線描画
	PlayerComboTimelineHelper::DrawTrackGridLines(context, trackTopY);

	// マウスがクリップ上か
	bool mouseOverClip = IsMouseOverAnyClip(context, steps, trackTopY);

	//===================================================================================================================
	// 空領域：クリックでCreate、Drag&Dropで追加
	//===================================================================================================================

	bool isRowHovered = false;
	bool isDroppedThisFrame = false;

	if (!mouseOverClip) {

		ImGui::PushID("ActionNodeTrackRow");

		ImVec2 rowPos = { context.contentScreenPos.x, trackTopY };
		ImGui::SetCursorScreenPos(rowPos);

		ImGui::InvisibleButton("##row_area",
			ImVec2((std::max)(1.0f, context.contentScreenSize.x), (std::max)(1.0f, context.view.trackHeight)));

		isRowHovered = ImGui::IsItemHovered();

		// Drag&Drop（空領域に落としたら追加）
		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PlayerComboTimelinePayload::kActionNodeAssetId)) {

				if (payload->DataSize == sizeof(uint32_t)) {

					uint32_t assetId = *static_cast<const uint32_t*>(payload->Data);

					float localX = (ImGui::GetIO().MousePos.x - context.contentScreenPos.x) + context.scroll.x - context.view.contentPadding.x;
					float t = (std::max)(0.0f, PlayerComboTimelineHelper::LocalXToTime(context.view, localX));
					t = PlayerComboTimelineHelper::SnapTime(t);

					context.model.AddStepAtTime(context.comboIndex, assetId, t, nullptr);
					context.model.SortStepsByStartTime(context.comboIndex);
					isDroppedThisFrame = true;
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::PopID();
	}

	//===================================================================================================================
	// クリップ表示
	//===================================================================================================================

	bool isAnyClipActiveThisFrame = false;
	//最初のステップ
	uint32_t firstStepId = 0;
	{

		float bestT = FLT_MAX;
		uint32_t bestId = 0;
		for (const auto& step : steps) {
			// 最も早い開始時間のステップを探す
			if (step.startTime < bestT || (step.startTime == bestT && step.stepId < bestId)) {
				bestT = step.startTime;
				bestId = step.stepId;
			}
		}
		// 最初のステップIDを保存
		firstStepId = bestId;
	}

	for (int32_t i = 0; i < static_cast<int32_t>(steps.size()); ++i) {

		auto& step = steps[static_cast<size_t>(i)];
		auto* asset = context.model.FindNodeAssetById(step.nodeAssetId);

		float x0 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, step.startTime) - context.scroll.x;

		// 実装がある場合は実装の合計時間を優先
		float duration = step.duration;
		if (asset && asset->implementation) {

			duration = (std::max)(0.0f, asset->implementation->GetTotalTime());
			step.duration = duration;
		}
		float x1 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(
			context.view, step.startTime + duration) - context.scroll.x;

		float y0 = trackTopY + 6.0f;
		float y1 = trackTopY + context.view.trackHeight - 6.0f;

		// 最低幅
		if ((x1 - x0) < 18.0f) {
			x1 = x0 + 18.0f;
		}

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

		ImGui::PushID(step.stepId);

		// クリック、ドラッグ領域
		ImGui::SetCursorScreenPos(p0);
		float w = (std::max)(1.0f, p1.x - p0.x);
		float h = (std::max)(1.0f, p1.y - p0.y);
		ImGui::InvisibleButton("##clip_btn", ImVec2(w, h));

		// 最初のアクションノードだけ、右クリックで開始入力設定
		bool isFirstClip = (step.stepId != 0 && step.stepId == firstStepId);
		if (isFirstClip && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {

			editComboId_ = combo.id;
			ImGui::OpenPopup("##ComboStartInputPopup");
		}
		// 開始入力設定ポップアップ
		if (ImGui::BeginPopup("##ComboStartInputPopup")) {

			ImGui::TextUnformatted("Combo Start Input (First Action Only)");

			auto* editCombo = context.model.FindComboById(editComboId_);
			if (editCombo) {

				DrawStepInputSettingUI(editCombo->startInput);
			} else {

				ImGui::TextUnformatted("Combo not found.");
			}

			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PlayerComboTimelinePayload::kActionNodeAssetId)) {

				if (payload->DataSize == sizeof(uint32_t)) {

					uint32_t assetId = *static_cast<const uint32_t*>(payload->Data);
					step.nodeAssetId = assetId;
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemActive()) {
			isAnyClipActiveThisFrame = true;
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

				int32_t index = context.model.FindStepIndexById(context.comboIndex, step.stepId);
				if (0 <= index) {
					context.model.RemoveComboStep(context.comboIndex, static_cast<size_t>(index));
				}
				context.select.selectedStepId = 0;
				context.select.selectedStepIndex = -1;
			}
			if (ImGui::MenuItem("Adjust Start")) {

				int32_t index = context.model.FindStepIndexById(context.comboIndex, step.stepId);
				// indexが1以上の時、前のステップの終了時間に合わせる
				if (1 <= index) {

					auto& prevStep = steps[static_cast<size_t>(index - 1)];
					step.startTime = prevStep.startTime + prevStep.duration;
				}
			}
			ImGui::EndPopup();
		}

		//===================================================================================================================
		// ドラッグ移動
		//===================================================================================================================

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			float deltaX = GetMouseDeltaX();
			float deltaT = deltaX / context.view.pixelsPerSecond;

			float newStart = step.startTime + deltaT;
			newStart = (std::max)(0.0f, newStart);
			newStart = PlayerComboTimelineHelper::SnapTime(newStart);

			step.startTime = newStart;
			isMovedThisFrame_ = true;
		}

		// 離したらソート
		if (isMovedThisFrame_ && ImGui::IsItemDeactivatedAfterEdit()) {

			uint32_t keepId = step.stepId;
			context.model.SortStepsByStartTime(context.comboIndex);

			int32_t newIndex = context.model.FindStepIndexById(context.comboIndex, keepId);
			context.select.selectedStepId = keepId;
			context.select.selectedStepIndex = newIndex;
			isMovedThisFrame_ = false;
		}

		//===================================================================================================================
		// ラベル：はみ出し防止 + ... 省略表示
		//===================================================================================================================
		{
			const float padX = 6.0f;
			const float padY = 6.0f;

			ImVec2 clipMin = ImVec2(p0.x + 2.0f, p0.y + 2.0f);
			ImVec2 clipMax = ImVec2(p1.x - 2.0f, p1.y - 2.0f);

			std::string label = asset != nullptr ? asset->name : "MissingNode";

			float availW = (clipMax.x - clipMin.x) - (padX * 2.0f);
			std::string drawText = MakeEllipsisText(label, availW);

			// 中央寄せ用の座標計算
			ImVec2 textSize = ImGui::CalcTextSize(drawText.c_str());

			ImVec2 areaSize = ImVec2(clipMax.x - clipMin.x, clipMax.y - clipMin.y);
			ImVec2 textPos = ImVec2(clipMin.x + (areaSize.x - textSize.x) * 0.5f,
				clipMin.y + (areaSize.y - textSize.y) * 0.5f);

			// クリップ内に収める
			textPos.x = (std::max)(textPos.x, clipMin.x + padX);
			textPos.y = (std::max)(textPos.y, clipMin.y + padY);

			context.drawList->PushClipRect(clipMin, clipMax, true);
			context.drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), drawText.c_str());
			context.drawList->PopClipRect();
		}

		ImGui::PopID();
	}

	//===================================================================================================================
	// 空クリックでCreate
	//===================================================================================================================
	{
		bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

		if (mouseReleased) {

			// dragged判定
			bool dragged = ImGui::IsMouseDragPastThreshold(ImGuiMouseButton_Left);

			// 空領域クリック判定
			if (!isDroppedThisFrame && isRowHovered && !dragged && !isAnyClipActiveThisFrame) {

				float localX = (ImGui::GetIO().MousePos.x - context.contentScreenPos.x) + context.scroll.x - context.view.contentPadding.x;
				float t = (std::max)(0.0f, PlayerComboTimelineHelper::LocalXToTime(context.view, localX));
				createTime_ = PlayerComboTimelineHelper::SnapTime(t);

				ImGui::OpenPopup("##CreateActionNodePopup");
			}
		}

		// 作成ポップアップ
		if (ImGui::BeginPopup("##CreateActionNodePopup")) {

			ImGui::TextUnformatted("Create ActionNode");
			SakuEngine::EnumAdapter<PlayerActionNodeType>::Combo("Type", &createType_);

			// 作成
			if (ImGui::Button("Create")) {

				uint32_t assetId = context.model.AddActionNode(createType_);
				context.model.AddStepAtTime(context.comboIndex, assetId, createTime_, nullptr);
				context.model.SortStepsByStartTime(context.comboIndex);

				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			// キャンセル
			if (ImGui::Button("Cancel")) {

				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}
#include "PlayerInputGraceTimelineTrack.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	PlayerInputGraceTimelineTrack classMethods
//============================================================================

void PlayerInputGraceTimelineTrack::DrawTrack(
	PlayerComboTimelineDrawContext& context, float trackTopY) {

	auto& combo = context.model.Combos()[context.comboIndex];
	auto& steps = combo.steps;

	// トラック背景
	ImU32 bg = IM_COL32(26, 26, 26, 255);
	ImVec2 a = { context.contentScreenPos.x, trackTopY };
	ImVec2 b = { context.contentScreenPos.x + context.contentScreenSize.x, trackTopY + context.view.trackHeight };
	context.drawList->AddRectFilled(a, b, bg);

	//===================================================================================================================
	// 猶予バーの描画
	//===================================================================================================================

	for (int32_t i = 0; i < static_cast<int32_t>(steps.size()); ++i) {

		auto& step = steps[static_cast<size_t>(i)];

		float graceStart = (std::max)(0.0f, step.inputGraceStartTime);
		float graceLen = (std::max)(0.0f, step.inputGraceTime);
		float graceEnd = graceStart + graceLen;

		float x0 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, graceStart) - context.scroll.x;
		float x1 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, graceEnd) - context.scroll.x;

		float y0 = trackTopY + 10.0f;
		float y1 = trackTopY + context.view.trackHeight - 10.0f;

		// 小さすぎると掴めないので最低幅
		if ((x1 - x0) < 6.0f) {
			x1 = x0 + 6.0f;
		}
		if (x1 < 0.0f || context.contentScreenSize.x < x0) {
			continue;
		}

		ImVec2 p0 = { context.contentScreenPos.x + x0, y0 };
		ImVec2 p1 = { context.contentScreenPos.x + x1, y1 };

		// 選択状態
		bool isSelected = (context.select.selectedStepId != 0 && context.select.selectedStepId == step.stepId);
		ImU32 color = isSelected ? IM_COL32(230, 170, 70, 255) : IM_COL32(180, 130, 50, 255);

		SakuEngine::ImGuiHelper::AddRectFilledRound(context.drawList, p0, p1, color, 6.0f);
		context.drawList->AddRect(p0, p1, IM_COL32(20, 20, 20, 255), 6.0f);

		//===================================================================================================================
		// 値表示
		//===================================================================================================================
		{
			char buf[64];
			std::snprintf(buf, sizeof(buf), "%.2f", step.inputGraceTime);
			context.drawList->AddText(ImVec2(p0.x + 6.0f, p0.y + 2.0f), IM_COL32(20, 20, 20, 255), buf);
		}

		ImGui::PushID(step.stepId);

		// ハンドル幅
		const float leftHandleW = 10.0f;
		const float rightHandleW = 10.0f;

		// 左ハンドル
		ImVec2 leftHandle0 = { p0.x, p0.y };
		ImVec2 leftHandle1 = { p0.x + leftHandleW, p1.y };
		SakuEngine::ImGuiHelper::AddRectFilledRound(context.drawList, leftHandle0, leftHandle1, IM_COL32(255, 255, 255, 120), 4.0f);
		ImGui::SetCursorScreenPos(leftHandle0);
		ImGui::InvisibleButton("##grace_left", ImVec2(leftHandleW, p1.y - p0.y));
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			// マウス移動量から時間変化を計算
			float deltaT = ImGui::GetIO().MouseDelta.x / context.view.pixelsPerSecond;
			// 終了時間は固定、開始時間を変更して長さを再計算
			float endTime = step.inputGraceStartTime + (std::max)(0.0f, step.inputGraceTime);
			float newStart = step.inputGraceStartTime + deltaT;
			newStart = (std::max)(0.0f, newStart);
			// 最低長さ確保
			float maxStart = (std::max)(0.0f, endTime - context.view.gridStep);
			newStart = (std::min)(maxStart, newStart);
			newStart = PlayerComboTimelineHelper::SnapTime(newStart);

			// 長さ再計算
			float newLen = endTime - newStart;
			newLen = (std::max)(0.0f, newLen);
			newLen = PlayerComboTimelineHelper::SnapTime(newLen);

			// 更新
			step.inputGraceStartTime = newStart;
			step.inputGraceTime = newLen;
		}

		// 右ハンドル
		ImVec2 rightHandle0 = { p1.x - rightHandleW, p0.y };
		ImVec2 rightHandle1 = { p1.x, p1.y };
		SakuEngine::ImGuiHelper::AddRectFilledRound(context.drawList, rightHandle0, rightHandle1, IM_COL32(255, 255, 255, 140), 4.0f);
		ImGui::SetCursorScreenPos(rightHandle0);
		ImGui::InvisibleButton("##grace_right", ImVec2(rightHandleW, p1.y - p0.y));
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			// マウス移動量から時間変化を計算
			float deltaT = ImGui::GetIO().MouseDelta.x / context.view.pixelsPerSecond;
			float newGrace = step.inputGraceTime + deltaT;
			newGrace = (std::max)(0.0f, newGrace);
			newGrace = PlayerComboTimelineHelper::SnapTime(newGrace);
			step.inputGraceTime = newGrace;
		}

		// 本体
		ImVec2 body0 = { p0.x + leftHandleW, p0.y };
		ImVec2 body1 = { p1.x - rightHandleW, p1.y };
		if (body1.x < body0.x) {
			body1.x = body0.x;
		}
		ImGui::SetCursorScreenPos(body0);
		ImGui::InvisibleButton("##grace_body", ImVec2(body1.x - body0.x, body1.y - body0.y));

		// 選択
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {

			context.select.selectedStepId = step.stepId;
			context.select.selectedStepIndex = i;

			editStepId_ = step.stepId;
			ImGui::OpenPopup("##StepInputSettingPopup");
		}

		// 本体ドラッグで開始位置変更
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			float deltaT = ImGui::GetIO().MouseDelta.x / context.view.pixelsPerSecond;
			float newStart = step.inputGraceStartTime + deltaT;
			newStart = (std::max)(0.0f, newStart);
			newStart = PlayerComboTimelineHelper::SnapTime(newStart);
			step.inputGraceStartTime = newStart;
		}

		// 右クリックメニュー
		if (ImGui::BeginPopupContextItem("##grace_context")) {

			if (ImGui::MenuItem("Reset Start To ActionEnd")) {
				step.inputGraceStartTime = step.startTime + step.duration;
				step.inputGraceStartTime = (std::max)(0.0f, PlayerComboTimelineHelper::SnapTime(step.inputGraceStartTime));
			}
			if (ImGui::MenuItem("Set Grace 0.0")) {
				step.inputGraceTime = 0.0f;
			}
			ImGui::EndPopup();
		}

		ImGui::PopID();
	}

	//===================================================================================================================
	// 設定ポップアップ
	//===================================================================================================================

	if (ImGui::BeginPopup("##StepInputSettingPopup")) {

		ImGui::TextUnformatted("StepInputSetting");

		int32_t idx = context.model.FindStepIndexById(context.comboIndex, editStepId_);
		if (0 <= idx) {

			auto& step = context.model.Combos()[context.comboIndex].steps[static_cast<size_t>(idx)];

			// 入力猶予はここでも調整可能
			ImGui::DragFloat("InputGraceStart(sec)", &step.inputGraceStartTime, 0.01f, 0.0f, 99.0f, "%.2f");
			ImGui::DragFloat("InputGraceTime(sec)", &step.inputGraceTime, 0.01f, 0.0f, 3.0f, "%.2f");

			ImGui::Separator();

			//===================================================================================================================
			// キーボード入力
			//===================================================================================================================

			ImGui::TextUnformatted("Keyboard");
			ImGui::Checkbox("UseKeyboard", &step.input.isUseKeyboard);

			if (step.input.isUseKeyboard) {

				// キー入力コンボ
				SakuEngine::EnumAdapter<KeyDIKCode>::Combo("Key", &step.input.keyDIKCode);

				ImGui::SameLine();
				if (ImGui::Button("Capture")) {

					isCaptureKeyboard_ = true;
				}

				if (isCaptureKeyboard_) {

					ImGui::TextUnformatted("Press any key");
					auto* input = SakuEngine::Input::GetInstance();
					for (auto v : magic_enum::enum_values<KeyDIKCode>()) {

						uint8_t raw = static_cast<uint8_t>(v);
						if (input->TriggerKey(static_cast<BYTE>(raw))) {

							step.input.keyDIKCode = v;
							isCaptureKeyboard_ = false;
							break;
						}
					}
					if (ImGui::Button("CancelCapture")) {

						isCaptureKeyboard_ = false;
					}
				}
			}

			ImGui::Separator();

			//===================================================================================================================
			// ゲームパッド入力
			//===================================================================================================================

			ImGui::TextUnformatted("GamePad");
			ImGui::Checkbox("UseGamePad", &step.input.isUseGamePad);

			if (step.input.isUseGamePad) {

				SakuEngine::EnumAdapter<GamePadButtons>::Combo("Button", &step.input.gamePadButton);

				ImGui::SameLine();
				if (ImGui::Button("Capture##Pad")) {

					isCapturePad_ = true;
				}
				if (isCapturePad_) {

					ImGui::TextUnformatted("Press any gamepad button...");
					auto* input = SakuEngine::Input::GetInstance();
					for (uint32_t bi = 0; bi < static_cast<uint32_t>(GamePadButtons::Counts); ++bi) {

						GamePadButtons button = static_cast<GamePadButtons>(bi);
						if (input->TriggerGamepadButton(button)) {

							step.input.gamePadButton = button;
							isCapturePad_ = false;
							break;
						}
					}
					if (ImGui::Button("CancelCapture##Pad")) {

						isCapturePad_ = false;
					}
				}
			}
		} else {

			ImGui::TextUnformatted("Step not found.");
		}
		ImGui::Separator();

		//===================================================================================================================
		// ポップアップを閉じる
		//===================================================================================================================

		if (ImGui::Button("Close")) {

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
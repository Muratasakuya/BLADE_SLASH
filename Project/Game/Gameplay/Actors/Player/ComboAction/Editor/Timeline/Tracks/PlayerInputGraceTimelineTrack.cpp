#include "PlayerInputGraceTimelineTrack.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

// imgui
#include <imgui.h>

//============================================================================
//	PlayerInputGraceTimelineTrack classMethods
//============================================================================

namespace {

	//===================================================================================================================
	//	猶予時間の制限
	//===================================================================================================================

	template <typename StepsT, typename StepT>
	inline float GetNextActionStartTime(const StepsT& steps, const StepT& self, float defaultValue) {

		float nextStart = defaultValue;
		for (const auto& step : steps) {
			if (step.stepId == self.stepId) {
				continue;
			}
			// 自分より後の最小startTimeを探す
			if (self.startTime < step.startTime) {

				nextStart = (std::min)(nextStart, step.startTime);
			}
		}
		return nextStart;
	}

	inline void ClampInputGraceRange(
		float actionMinTime, float actionMaxTime,
		float& graceStartTime, float& graceDuration) {

		// durationは0.0f以上
		graceDuration = (std::max)(0.0f, graceDuration);

		// Startを範囲内へ
		graceStartTime = (std::max)(actionMinTime, graceStartTime);
		graceStartTime = (std::min)(actionMaxTime, graceStartTime);

		// Endを範囲内へ
		float endTime = graceStartTime + graceDuration;
		endTime = (std::min)(actionMaxTime, endTime);
		endTime = (std::max)(graceStartTime, endTime);
		graceDuration = endTime - graceStartTime;
	}
}


void PlayerInputGraceTimelineTrack::DrawTrack(
	PlayerComboTimelineDrawContext& context, float trackTopY) {

	auto& combo = context.model.Combos()[context.comboIndex];
	auto& steps = combo.steps;

	// トラック背景
	ImU32 bg = IM_COL32(26, 26, 26, 255);
	ImVec2 a = { context.contentScreenPos.x, trackTopY };
	ImVec2 b = { context.contentScreenPos.x + context.contentScreenSize.x, trackTopY + context.view.trackHeight };
	context.drawList->AddRectFilled(a, b, bg);

	// グリッド線描画
	PlayerComboTimelineHelper::DrawTrackGridLines(context, trackTopY);

	//===================================================================================================================
	// 猶予バーの描画
	//===================================================================================================================

	for (int32_t i = 0; i < static_cast<int32_t>(steps.size()); ++i) {

		auto& step = steps[static_cast<size_t>(i)];

		//---------------------------------------------------------------------------------------------------------------
		// 入力猶予の有効範囲、自分のアクションより前に行かない + 次のアクションの開始より先に行かない
		//---------------------------------------------------------------------------------------------------------------

		// 自身のアクションより前に行かない
		float minTime = step.startTime;
		// 次のアクションまでの時間を最大にする
		float maxTime = GetNextActionStartTime(steps, step, context.totalTime);
		maxTime = (std::max)(minTime, maxTime);

		// モデル側に入力猶予開始時間を持たせる
		float& graceStartTime = step.inputGraceStartTime;
		float& graceDuration = step.inputGraceTime;

		// 入力猶予時間を制限する
		ClampInputGraceRange(minTime, maxTime, graceStartTime, graceDuration);

		float graceEndTime = graceStartTime + graceDuration;

		//---------------------------------------------------------------------------------------------------------------
		// 表示座標へ変換
		//---------------------------------------------------------------------------------------------------------------
		float x0 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, graceStartTime) - context.scroll.x;
		float x1 = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, graceEndTime) - context.scroll.x;

		float y0 = trackTopY + 10.0f;
		float y1 = trackTopY + context.view.trackHeight - 10.0f;

		// ハンドルを置ける最低幅
		constexpr float kHandleW = 10.0f;
		constexpr float kMinBarW = kHandleW * 2.0f + 4.0f;
		if ((x1 - x0) < kMinBarW) {
			x1 = x0 + kMinBarW;
		}

		// 画面外は省略
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

		ImGui::PushID(step.stepId);

		//---------------------------------------------------------------------------------------------------------------
		// クリック領域
		//---------------------------------------------------------------------------------------------------------------
		float barH = (std::max)(1.0f, p1.y - p0.y);

		// 左ハンドル
		ImVec2 left0 = { p0.x, p0.y };
		ImVec2 left1 = { p0.x + kHandleW, p1.y };
		SakuEngine::ImGuiHelper::AddRectFilledRound(context.drawList, left0, left1, IM_COL32(255, 255, 255, 140), 4.0f);
		ImGui::SetCursorScreenPos(left0);
		ImGui::InvisibleButton("##grace_left", ImVec2((std::max)(1.0f, left1.x - left0.x), barH));

		// 左ハンドル、開始時間変更
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			auto* input = SakuEngine::Input::GetInstance();
			float deltaX = input != nullptr ? input->GetMouseMoveValue().x : ImGui::GetIO().MouseDelta.x;
			float deltaT = deltaX / context.view.dragsSensitivity;

			float endFixed = graceStartTime + graceDuration;
			float newStart = graceStartTime + deltaT;
			newStart = PlayerComboTimelineHelper::SnapTime(newStart);

			// Clamp
			newStart = (std::max)(minTime, newStart);
			newStart = (std::min)(endFixed, newStart);
			endFixed = (std::min)(maxTime, endFixed);
			if (endFixed < newStart) {
				endFixed = newStart;
			}

			graceStartTime = newStart;
			graceDuration = endFixed - newStart;
			ClampInputGraceRange(minTime, maxTime, graceStartTime, graceDuration);
		}

		// 右ハンドル
		ImVec2 right0 = { p1.x - kHandleW, p0.y };
		ImVec2 right1 = { p1.x, p1.y };
		SakuEngine::ImGuiHelper::AddRectFilledRound(context.drawList, right0, right1, IM_COL32(255, 255, 255, 140), 4.0f);
		ImGui::SetCursorScreenPos(right0);
		ImGui::InvisibleButton("##grace_right", ImVec2((std::max)(1.0f, right1.x - right0.x), barH));

		// 右ハンドル、終了時間変更
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			auto* input = SakuEngine::Input::GetInstance();
			float deltaX = input != nullptr ? input->GetMouseMoveValue().x : ImGui::GetIO().MouseDelta.x;
			float deltaT = deltaX / context.view.dragsSensitivity;

			float newEnd = graceStartTime + graceDuration + deltaT;
			newEnd = PlayerComboTimelineHelper::SnapTime(newEnd);
			newEnd = (std::max)(graceStartTime, newEnd);
			newEnd = (std::min)(maxTime, newEnd);

			graceDuration = newEnd - graceStartTime;
			ClampInputGraceRange(minTime, maxTime, graceStartTime, graceDuration);
		}

		// 本体
		ImVec2 body0 = { p0.x + kHandleW, p0.y };
		ImVec2 body1 = { p1.x - kHandleW, p1.y };
		float bodyW = (std::max)(1.0f, body1.x - body0.x);
		ImGui::SetCursorScreenPos(body0);
		ImGui::InvisibleButton("##grace_body", ImVec2(bodyW, barH));

		// 選択
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			context.select.selectedStepId = step.stepId;
			context.select.selectedStepIndex = i;
		}

		// ドラッグ移動
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			auto* input = SakuEngine::Input::GetInstance();
			float deltaX = input != nullptr ? input->GetMouseMoveValue().x : ImGui::GetIO().MouseDelta.x;
			float deltaT = deltaX / context.view.dragsSensitivity;

			float dur = graceDuration;
			float maxStart = maxTime - dur;
			maxStart = (std::max)(minTime, maxStart);

			float newStart = graceStartTime + deltaT;
			newStart = PlayerComboTimelineHelper::SnapTime(newStart);
			newStart = (std::max)(minTime, newStart);
			newStart = (std::min)(maxStart, newStart);

			graceStartTime = newStart;
			ClampInputGraceRange(minTime, maxTime, graceStartTime, graceDuration);
		}

		// クリックで設定表示
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			editStepId_ = step.stepId;
			ImGui::OpenPopup("##StepInputSettingPopup");
		}

		// 右クリック、メニュー
		if (ImGui::BeginPopupContextItem("##grace_context")) {
			if (ImGui::MenuItem("Set Grace 0.0")) {
				graceDuration = 0.0f;
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

			// 入力猶予の制限
			float minTime = step.startTime;
			float maxTime = GetNextActionStartTime(context.model.Combos()[context.comboIndex].steps, step, context.totalTime);
			maxTime = (std::max)(minTime, maxTime);

			float& graceStartTime = step.inputGraceStartTime;
			float& graceDuration = step.inputGraceTime;
			ClampInputGraceRange(minTime, maxTime, graceStartTime, graceDuration);

			// Start
			ImGui::DragFloat("GraceStart(sec)", &graceStartTime, 0.01f, minTime, maxTime, "%.2f");
			ClampInputGraceRange(minTime, maxTime, graceStartTime, graceDuration);

			// End
			float endTime = graceStartTime + graceDuration;
			if (ImGui::DragFloat("GraceEnd(sec)", &endTime, 0.01f, minTime, maxTime, "%.2f")) {
				endTime = (std::max)(graceStartTime, endTime);
				endTime = (std::min)(maxTime, endTime);
				graceDuration = endTime - graceStartTime;
				ClampInputGraceRange(minTime, maxTime, graceStartTime, graceDuration);
			}

			// Duration
			ImGui::DragFloat("GraceDuration(sec)", &graceDuration, 0.01f, 0.0f, (std::max)(0.0f, maxTime - graceStartTime), "%.2f");
			ClampInputGraceRange(minTime, maxTime, graceStartTime, graceDuration);

			ImGui::Separator();

			//===================================================================================================================
			// キーボード入力
			//===================================================================================================================

			ImGui::TextUnformatted("Keyboard");
			ImGui::Checkbox("UseKeyboard", &step.input.isUseKeyboard);

			if (step.input.isUseKeyboard) {

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

		if (ImGui::Button("Close")) {

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
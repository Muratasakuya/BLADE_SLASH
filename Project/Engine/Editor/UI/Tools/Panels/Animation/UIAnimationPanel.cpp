#include "UIAnimationPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/UIAnimationLibrary.h>

#include <imgui_internal.h>

//============================================================================
//	UIAnimationPanel classMethods
//============================================================================

void UIAnimationPanel::ImGui(UIToolContext& context) {

	ImGui::SetWindowFontScale(0.72f);

	UIAnimationLibrary* lib = context.animationLibrary;
	if (!lib) {
		ImGui::TextUnformatted("No UIAnimationLibrary.");
		ImGui::SetWindowFontScale(1.0f);
		return;
	}

	const ImVec2 itemSize = ImVec2(200.0f, 22.0f);

	//============================================================================
	//	左右分割（左：作成+一覧 / 右：詳細）
	//============================================================================w

	ImGuiTableFlags splitFlags = ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;

	if (ImGui::BeginTable("##UIAnimationPanelSplit", 2, splitFlags)) {

		ImGui::TableSetupColumn("List", ImGuiTableColumnFlags_WidthFixed, 320.0f);
		ImGui::TableSetupColumn("Detail", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		//========================================================================
		// Left: Create + List
		//========================================================================
		ImGui::TableSetColumnIndex(0);

		// 左側を一つの枠に
		if (ImGui::BeginChild("##AnimPanelLeft", ImVec2(0.0f, 0.0f), true)) {

			ImGui::SeparatorText("Create");
			// デフォルト名セット
			if (newName_.inputText.empty()) {

				newName_.inputText = "Animation";
				newName_.CopyToBuffer();
			}

			// 初期設定
			ImGui::SetNextItemWidth(itemSize.x);
			ImGuiHelper::InputText("name##newAnimation", newName_);
			ImGui::SetNextItemWidth(itemSize.x);
			EnumAdapter<CanvasType>::Combo("canvas##newAnimation", &newCanvasType_);

			// 作成ボタン
			if (ImGui::Button("Create##newAnimation", itemSize)) {

				UIAnimationHandle handle = lib->Create(newCanvasType_, newName_.inputText);
				if (auto* entry = lib->GetEntry(handle)) {

					selectedUid_ = entry->clip.uid;
				}
			}

			ImGui::Spacing();
			ImGui::SeparatorText("Clips");

			const float listH = ImGui::GetContentRegionAvail().y;
			if (ImGui::BeginChild("##AnimList", ImVec2(0.0f, listH), true)) {

				ImGuiTableFlags listFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
					ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX;

				if (ImGui::BeginTable("##AnimListTable", 2, listFlags)) {

					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("UID", ImGuiTableColumnFlags_WidthFixed, 72.0f);

					lib->ForEachClips([&](UIAnimationHandle, UIAnimationEntry& entry) {

						const bool selected = (entry.clip.uid == selectedUid_);

						ImGui::TableNextRow();
						ImGui::PushID(entry.clip.uid);

						ImGui::TableSetColumnIndex(0);

						// 行高固定
						ImGuiSelectableFlags sflags = ImGuiSelectableFlags_SpanAllColumns;
						if (ImGui::Selectable(entry.clip.name.c_str(), selected, sflags, ImVec2(0.0f, itemSize.y))) {

							selectedUid_ = entry.clip.uid;
						}

						// ここからドラッグ開始
						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

							const uint32_t uid = entry.clip.uid;
							ImGui::SetDragDropPayload(kAnimationPayloadId, &uid, sizeof(uint32_t));

							ImGui::Text("Clip: %s", entry.clip.name.c_str());
							ImGui::Text("uid: %u", entry.clip.uid);

							ImGui::EndDragDropSource();
						}

						// 右クリックメニュー
						if (ImGui::BeginPopupContextItem()) {

							if (ImGui::MenuItem("Delete")) {

								lib->Destroy(entry.clip.uid);
								if (selectedUid_ == entry.clip.uid) {
									selectedUid_ = 0;
									lastSelectedUid_ = 0;
								}
							}
							ImGui::EndPopup();
						}
						// UID col
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%u", entry.clip.uid);
						ImGui::PopID();
						});
					ImGui::EndTable();
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild(); // left

		//========================================================================
		// Right: Selected Detail
		//========================================================================

		ImGui::TableSetColumnIndex(1);

		if (ImGui::BeginChild("##AnimPanelRight", ImVec2(0.0f, 0.0f), true)) {

			UIAnimationClip* clip = lib->GetClip(selectedUid_);
			if (!clip) {

				ImGui::TextUnformatted("No clip selected.");
				ImGui::EndChild();
				ImGui::EndTable();
				ImGui::SetWindowFontScale(1.0f);
				return;
			}

			// 選択が変わったらrenameバッファ同期
			if (lastSelectedUid_ != selectedUid_) {

				rename_.inputText = clip->name;
				rename_.CopyToBuffer();
				lastSelectedUid_ = selectedUid_;
			}

			ImGui::Spacing();
			ImGui::SeparatorText("Name");

			ImGui::SetNextItemWidth(200.0f);
			ImGuiHelper::InputText("name##rename", rename_);

			// ボタンを右側で揃える
			const float avail = ImGui::GetContentRegionAvail().x;
			const float buttonWidth = (avail - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

			if (ImGui::Button("Apply Rename##rename", ImVec2(buttonWidth, itemSize.y))) {

				lib->Rename(clip->uid, rename_.inputText);

				// ユニーク化されて名前が変わる可能性があるので再同期
				if (auto* updated = lib->GetClip(clip->uid)) {
					rename_.inputText = updated->name;
					rename_.CopyToBuffer();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete##selected", ImVec2(buttonWidth, itemSize.y))) {

				lib->Destroy(clip->uid);
				selectedUid_ = 0;
				lastSelectedUid_ = 0;
			}

			ImGui::Spacing();
			ImGui::Separator();

			// 選択中のクリップ編集

			// 全てのアニメーショントラックを表示し、編集

		}
		ImGui::EndChild(); // right
		ImGui::EndTable();
	}

	ImGui::SetWindowFontScale(1.0f);
}
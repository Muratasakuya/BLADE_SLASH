#include "UIWidgetPalette.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/UIWidgetEditorContext.h>
#include <Engine/Editor/UI/Editor/Document/UIWidgetTypeRegistry.h>
#include <Engine/Editor/UI/Editor/Document/UIWidgetDocument.h>

// imgui
#include <imgui.h>

//============================================================================
//	UIWidgetPalette classMethods
//============================================================================

namespace {

	// 検索を通過するかどうか
	bool PassSearch(const char* search, const std::string& string) {

		// 空文字列はすべて通過
		if (!search || search[0] == '\0') {
			return true;
		}
		std::string ss(search);
		// 大文字小文字は簡易に
		return (string.find(ss) != std::string::npos);
	}
}

void UIWidgetPalette::Draw(UIWidgetEditorContext& context) {

	ImGui::InputText("Search", searchBuf_, sizeof(searchBuf_));

	ImGui::Separator();

	// 全ウィジェット型取得
	const auto& all = context.registry->GetAll();

	// カテゴリーごとに表示
	std::unordered_map<std::string, std::vector<const UIWidgetTypeInfo*>> buckets;
	for (auto& info : all) {
		// 検索がかかっていない&&通過しないものはスキップ
		if (!PassSearch(searchBuf_, info.displayName) && !PassSearch(searchBuf_, info.type)) {
			continue;
		}
		buckets[info.category].push_back(&info);
	}

	//=====================================================================================================================
	//	カテゴリーごとに区切って表示
	//=====================================================================================================================

	for (auto& bucket : buckets) {
		
		//=====================================================================================================================
		//	見出し
		//=====================================================================================================================
		if (ImGui::CollapsingHeader(bucket.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			for (auto* info : bucket.second) {

				ImGui::PushID(info->type.c_str());
				if (ImGui::Selectable(info->displayName.c_str())) {

					// 選択中がPanelならそこ、違うならroot
					uint32_t parentId = context.document->GetRootId();
					if (context.selection.HasSelection()) {

						// 現在選択中ノード取得
						const UIWidgetNode* selectNode = context.document->FindNode(context.selection.selectedNodeId);
						if (selectNode) {

							const UIWidgetTypeInfo* selType = context.registry->Find(selectNode->typeName);
							if (selType && selType->isPanel) {

								parentId = selectNode->id;
							}
						}
					}

					// ノード追加
					uint32_t newId = context.document->AddNode(info->type, parentId, info->type);
					if (newId != 0) {

						context.selection.selectedNodeId = newId;
						context.MarkDocumentDirty(true);
						context.RequestPreviewRebuild();
					}
				}

				//=====================================================================================================================
				//	ドラッグ＆ドロップのソース
				//=====================================================================================================================

				if (ImGui::BeginDragDropSource()) {

					ImGui::SetDragDropPayload("UIWIDGET_TYPE", info->type.c_str(), info->type.size() + 1);
					ImGui::Text("%s", info->displayName.c_str());
					ImGui::EndDragDropSource();
				}
				ImGui::PopID();
			}
		}
	}
}
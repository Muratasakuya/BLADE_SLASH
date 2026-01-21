#include "UIWidgetHierarchy.h"

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
//	UIWidgetHierarchy classMethods
//============================================================================

void UIWidgetHierarchy::Draw(UIWidgetEditorContext& context) {

	// ドキュメントがない場合は何もしない
	if (!context.document) {
		return;
	}

	// ルートノードから描画
	const uint32_t rootId = context.document->GetRootId();
	DrawNode(context, rootId);

	if (context.selection.HasSelection()) {

		ImGui::Separator();
		if (ImGui::Button("Delete Selected")) {

			uint32_t deleteId = context.selection.selectedNodeId;
			// rootは削除不可
			if (deleteId != rootId) {

				context.document->RemoveNode(deleteId);
				context.selection.Clear();
				context.MarkDocumentDirty(true);
				context.RequestPreviewRebuild();
			}
		}
	}
}

void UIWidgetHierarchy::DrawNode(UIWidgetEditorContext& context, uint32_t nodeId) {
	
	UIWidgetNode* node = context.document->FindNode(nodeId);
	if (!node) {
		return;
	}

	//=====================================================================================================================
	//	表示フラグ設定
	//=====================================================================================================================

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
	if (node->children.empty()) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	if (context.selection.selectedNodeId == nodeId) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	std::string label = node->name + " (" + node->typeName + ")";
	bool open = ImGui::TreeNodeEx((void*)(uintptr_t)nodeId, flags, "%s", label.c_str());

	if (ImGui::IsItemClicked()) {

		context.selection.selectedNodeId = nodeId;
	}

	//=====================================================================================================================
	//	ノードを別親へ移動
	//=====================================================================================================================

	if (ImGui::BeginDragDropSource()) {

		ImGui::SetDragDropPayload("UIWIDGET_NODE", &nodeId, sizeof(uint32_t));
		ImGui::Text("%s", node->name.c_str());
		ImGui::EndDragDropSource();
	}

	//=====================================================================================================================
	//	ノードを別親へ移動、または新規ノード追加のドロップ先
	//=====================================================================================================================

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("UIWIDGET_NODE")) {

			uint32_t draggedId = *(const uint32_t*)payload->Data;
			// drop先がpanelならreparent
			const UIWidgetNode* target = context.document->FindNode(nodeId);
			if (target && context.registry) {

				// パネルかどうか確認
				const UIWidgetTypeInfo* info = context.registry->Find(target->typeName);
				if (info && info->isPanel) {
					if (context.document->ReparentNode(draggedId, nodeId)) {

						context.MarkDocumentDirty(true);
						context.RequestPreviewRebuild();
					}
				}
			}
		}

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("UIWIDGET_TYPE")) {

			const char* type = (const char*)payload->Data;
			const UIWidgetNode* target = context.document->FindNode(nodeId);
			if (target && context.registry) {

				// パネルかどうか確認
				const UIWidgetTypeInfo* info = context.registry->Find(target->typeName);
				if (info && info->isPanel) {

					// ノード追加
					uint32_t newId = context.document->AddNode(type, nodeId, type);
					if (newId != 0) {

						context.selection.selectedNodeId = newId;
						context.MarkDocumentDirty(true);
						context.RequestPreviewRebuild();
					}
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	//=====================================================================================================================
	//	子ノード描画
	//=====================================================================================================================

	if (open) {

		for (uint32_t child : node->children) {

			DrawNode(context, child);
		}
		ImGui::TreePop();
	}
}
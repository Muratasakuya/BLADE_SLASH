#include "UIWidgetVisualDesigner.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/UIWidgetEditorContext.h>
#include <Engine/Editor/UI/Editor/Preview/UIWidgetPreviewRuntime.h>
#include <Engine/Editor/UI/Editor/Document/UIWidgetDocument.h>

//============================================================================
//	UIWidgetVisualDesigner classMethods
//============================================================================

void UIWidgetVisualDesigner::Init(const D3D12_GPU_DESCRIPTOR_HANDLE& renderTextureGPUHandle) {

	// GPUハンドル保存
	renderTextureGPUHandle_ = renderTextureGPUHandle;
}

void UIWidgetVisualDesigner::Draw(UIWidgetEditorContext& context) {

	if (!context.preview || !context.document) {
		ImGui::TextDisabled("Preview not ready.");
		return;
	}

	//=====================================================================================================================
	//	描画結果表示
	//=====================================================================================================================

	ImGui::Image(ImTextureID(renderTextureGPUHandle_.ptr), gameViewSize_);

	// viewportを合わせる
	context.preview->SetViewportSize(Vector2(gameViewSize_.x, gameViewSize_.y));

	// 表示画像スクリーン座標
	ImVec2 imageMin = ImGui::GetItemRectMin();
	ImVec2 imageMax = ImGui::GetItemRectMax();

	// マウス座標をviewport内へ
	ImVec2 mousePos = ImGui::GetMousePos();
	Vector2 localMouse{ mousePos.x - imageMin.x, mousePos.y - imageMin.y };

	// ホバー状態取得
	const bool hovered = ImGui::IsItemHovered();

	//=====================================================================================================================
	//	クリックでピック、選択矩形オーバーレイ、ドラッグ移動
	//=====================================================================================================================

	// クリックでピック
	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

		auto pick = context.preview->PickWidgetAt(localMouse);
		if (pick.valid) {

			context.selection.selectedNodeId = pick.widgetId;
		}
	}

	// 選択矩形オーバーレイ
	if (context.selection.HasSelection()) {

		Vector2 pos{}, size{};
		if (context.preview->GetWidgetRect(context.selection.selectedNodeId, pos, size)) {

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 a(imageMin.x + pos.x, imageMin.y + pos.y);
			ImVec2 b(imageMin.x + pos.x + size.x, imageMin.y + pos.y + size.y);
			drawList->AddRect(a, b, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);

			// ドラッグ移動
			UIWidgetNode* node = context.document->FindNode(context.selection.selectedNodeId);
			if (node && hovered) {

				// 選択矩形内にマウスがあるか
				bool inside = (localMouse.x >= pos.x && localMouse.y >= pos.y &&
					localMouse.x <= pos.x + size.x && localMouse.y <= pos.y + size.y);
				if (!isDragging_ && inside && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

					isDragging_ = true;
					dragStartMouse_ = localMouse;
					dragStartOffsetsLT_ = Vector2(node->layout.offsets.left, node->layout.offsets.top);
				}
				if (isDragging_ && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {

					Vector2 delta = localMouse - dragStartMouse_;

					// 非ストレッチ前提で移動
					node->layout.offsets.left = dragStartOffsetsLT_.x + delta.x;
					node->layout.offsets.top = dragStartOffsetsLT_.y + delta.y;

					context.MarkDocumentDirty(true);
					context.syncPreviewRequested = true;
				}
				if (isDragging_ && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {

					isDragging_ = false;
				}
			}
		}
	}
}
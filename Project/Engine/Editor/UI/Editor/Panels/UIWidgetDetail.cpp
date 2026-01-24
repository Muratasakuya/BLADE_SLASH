#include "UIWidgetDetail.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/UIWidgetEditorContext.h>
#include <Engine/Editor/UI/Editor/Document/UIWidgetDocument.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	UIWidgetDetail classMethods
//============================================================================

bool UIWidgetDetail::EditMargin(const char* label, float& left, float& top, float& right, float& bottom) {

	// arrayにまとめてDragFloat4で編集
	float array[4] = { left, top, right, bottom };
	bool changed = ImGui::DragFloat4(label, array, 0.1f);
	if (changed) {

		left = array[0];
		top = array[1];
		right = array[2];
		bottom = array[3];
	}
	return changed;
}

void UIWidgetDetail::Draw(UIWidgetEditorContext& context) {

	if (!context.document || !context.selection.HasSelection()) {

		ImGui::TextDisabled("No selection.");
		return;
	}

	// 選択ノード取得
	UIWidgetNode* node = context.document->FindNode(context.selection.selectedNodeId);
	if (!node) {
		ImGui::TextDisabled("Invalid selection.");
		return;
	}

	ImGui::Text("Selected: %s", node->name.c_str());
	ImGui::Separator();

	// 共通部分の描画
	DrawCommon(context, node->id);
	ImGui::Separator();
	// 型固有部分の描画
	DrawTypeSpecific(context, node->id, node->typeName);
}

void UIWidgetDetail::DrawCommon(UIWidgetEditorContext& context, uint32_t id) {

	UIWidgetNode* node = context.document->FindNode(id);
	if (!node) {
		return;
	}

	//=====================================================================================================================
	//	名前の変更
	//=====================================================================================================================

	char buf[256]{};
	std::snprintf(buf, sizeof(buf), "%s", node->name.c_str());
	if (ImGui::InputText("Name", buf, sizeof(buf))) {
		if (context.document->RenameNode(id, buf)) {

			context.MarkDocumentDirty(true);
			context.RequestPreviewRebuild();
		}
	}

	//=====================================================================================================================
	//	表示フラグ設定
	//=====================================================================================================================

	if (ImGui::Checkbox("Enabled", &node->enabled)) {

		context.MarkDocumentDirty(true);
		context.syncPreviewRequested = true;
	}

	//=====================================================================================================================
	//	可視性設定
	//=====================================================================================================================

	if (EnumAdapter<UIVisibility>::Combo("Visibility", &node->visibility)) {

		context.MarkDocumentDirty(true);
		context.syncPreviewRequested = true;
	}

	//=====================================================================================================================
	//	レイアウト設定
	//=====================================================================================================================

	if (ImGui::CollapsingHeader("Layout", ImGuiTreeNodeFlags_DefaultOpen)) {

		bool changed = false;
		changed |= ImGui::DragFloat2("Anchors Min", &node->layout.anchors.min.x, 0.01f);
		changed |= ImGui::DragFloat2("Anchors Max", &node->layout.anchors.max.x, 0.01f);
		changed |= EditMargin("Offsets (L,T,R,B)", node->layout.offsets.left, node->layout.offsets.top, node->layout.offsets.right, node->layout.offsets.bottom);
		changed |= ImGui::DragFloat2("Alignment", &node->layout.alignment.x, 0.01f);
		if (changed) {

			context.MarkDocumentDirty(true);
			context.syncPreviewRequested = true;
		}
	}
}

void UIWidgetDetail::DrawTypeSpecific(UIWidgetEditorContext& context, uint32_t id, const std::string& type) {

	UIWidgetNode* node = context.document->FindNode(id);
	if (!node) {
		return;
	}

	//=====================================================================================================================
	//	Image
	//=====================================================================================================================

	// Image
	if (type == "Image") {

		// ここでテクスチャの変更など...

		return;
	}

	//=====================================================================================================================
	//	TextBlock
	//=====================================================================================================================

	if (type == "TextBlock") {

		std::string text = context.document->GetProperty<std::string>(id, "text", "Text");
		char textBuffer[512]{};
		std::snprintf(textBuffer, sizeof(textBuffer), "%s", text.c_str());
		// マルチライン入力
		if (ImGui::InputTextMultiline("Text", textBuffer, sizeof(textBuffer), ImVec2(-1.0f, 80.0f))) {

			context.document->SetProperty(id, "text", std::string(textBuffer));
			context.MarkDocumentDirty(true);
			context.syncPreviewRequested = true;
		}

		std::string bind = context.document->GetProperty<std::string>(id, "textBindingKey", "");
		char bindBuffer[256]{};
		std::snprintf(bindBuffer, sizeof(bindBuffer), "%s", bind.c_str());
		// バインド入力
		if (ImGui::InputText("Binding Key", bindBuffer, sizeof(bindBuffer))) {

			context.document->SetProperty(id, "textBindingKey", std::string(bindBuffer));
			context.MarkDocumentDirty(true);
			context.syncPreviewRequested = true;
		}
		return;
	}

	//=====================================================================================================================
	//	TextBlock
	//=====================================================================================================================

	if (type == "Button") {

		// 共通の編集関数
		auto editStr = [&](const char* label, const char* key) {

			std::string v = context.document->GetProperty<std::string>(id, key, "");
			char buffer[256]{};
			std::snprintf(buffer, sizeof(buffer), "%s", v.c_str());
			// 入力
			if (ImGui::InputText(label, buffer, sizeof(buffer))) {

				context.document->SetProperty(id, key, std::string(buffer));
				context.MarkDocumentDirty(true);
				context.syncPreviewRequested = true;
			}
			};

		editStr("Action Key", "clickActionKey");
		editStr("Style Normal", "style.normal");
		editStr("Style Hovered", "style.hovered");
		editStr("Style Pressed", "style.pressed");
		editStr("Style Disabled", "style.disabled");
		return;
	}

	//=====================================================================================================================
	//	Slider
	//=====================================================================================================================

	if (type == "Slider") {

		float value = context.document->GetProperty<float>(id, "value", 0.0f);
		if (ImGui::SliderFloat("Value", &value, 0.0f, 1.0f)) {

			context.document->SetProperty(id, "value", value);
			context.MarkDocumentDirty(true);
			context.syncPreviewRequested = true;
		}

		std::string bind = context.document->GetProperty<std::string>(id, "valueBindingKey", "");
		char buffer[256]{};
		std::snprintf(buffer, sizeof(buffer), "%s", bind.c_str());
		// バインド入力
		if (ImGui::InputText("Binding Key", buffer, sizeof(buffer))) {

			context.document->SetProperty(id, "valueBindingKey", std::string(buffer));
			context.MarkDocumentDirty(true);
			context.syncPreviewRequested = true;
		}
		return;
	}

	ImGui::TextDisabled("No type-specific properties.");
}
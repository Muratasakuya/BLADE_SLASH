#include "UIWidgetDocumentsPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/UIWidgetEditor.h>

//============================================================================
//	UIWidgetDocumentsPanel classMethods
//============================================================================

void UIWidgetDocumentsPanel::Draw([[maybe_unused]] UIWidgetEditorContext& context) {

	// アクティブドキュメント表示
	ImGui::Text("Active: %s", context.editor->GetActiveKey().c_str());
	ImGui::Separator();

	ImGui::InputText("Key", newKey_, sizeof(newKey_));
	if (ImGui::Button("Create")) {

		context.editor->CreateDocument(std::string(newKey_));
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("All Documents");

	// 一覧
	for (const auto& document : context.editor->GetDocuments()) {

		// アクティブかどうか
		const std::string& key = document.first;
		bool isActive = (key == context.editor->GetActiveKey());

		ImGui::PushID(key.c_str());

		if (ImGui::Selectable(key.c_str(), isActive)) {

			context.editor->SetActiveDocument(key);
		}

		ImGui::SameLine();
		ImGui::BeginDisabled(isActive && context.editor->GetDocuments().size() == 1);
		if (ImGui::SmallButton("Delete")) {

			context.editor->DeleteDocument(key);
			ImGui::PopID();
			// 削除されたらループを抜ける
			break;
		}
		ImGui::EndDisabled();
		ImGui::PopID();
	}
}
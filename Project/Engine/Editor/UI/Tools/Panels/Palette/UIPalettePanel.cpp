#include "UIPalettePanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UIPalettePanel classMethods
//============================================================================

void UIPalettePanel::ImGui(UIToolContext& context) {

	UIAsset* asset = context.GetSelectedAsset();
	if (!asset) {
		ImGui::Text("No UIAsset selected.");
		return;
	}

	ImGui::SetWindowFontScale(0.8f);

	// 追加先の親要素を表示
	auto parent = context.selectedElement.IsValid() ?
		context.selectedElement : asset->rootHandle;

	ImGui::Text("Add to: %s", asset->Get(parent)->name.c_str());

	// 親矩形トランスフォームを自動で追加するか
	bool autoAddParent = context.paletteRegistry->IsAutoAddParentRectTransform();
	if (ImGui::Checkbox("Auto Add Parent", &autoAddParent)) {

		context.paletteRegistry->SetAutoAddParentRectTransform(autoAddParent);
	}

	ImGui::Separator();

	// カテゴリごとに表示
	DrawCategory(context, UIPaletteItemCategory::Panel);
	DrawCategory(context, UIPaletteItemCategory::Leaf);

	ImGui::SetWindowFontScale(1.0f);
}

void UIPalettePanel::DrawCategory(UIToolContext& context, UIPaletteItemCategory category) {

	// 折りたたみヘッダー
	if (!ImGui::CollapsingHeader(EnumAdapter<UIPaletteItemCategory>::ToString(category))) {
		return;
	}

	for (auto& item : context.paletteRegistry->GetItems()) {

		// カテゴリが違う場合はスキップ
		if (item.category != category) {
			continue;
		}

		// アイテム名
		const char* itemName = EnumAdapter<UIPaletteItemType>::ToString(item.type);

		// アイテム表示
		ImGui::Image(ImTextureID(context.asset->GetGPUHandle(item.iconName).ptr), ImVec2(24.0f, 24.0f));
		ImGui::SameLine();
		ImGui::Selectable(itemName, false);

		// ドラッグソースの開始
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

			// ペイロードにアイテムタイプを設定
			UIPaletteItemType payloadType = item.type;
			ImGui::SetDragDropPayload(kPalettePayloadId, &payloadType, sizeof(payloadType));

			ImGui::Text("Create : %s", itemName);
			ImGui::EndDragDropSource();
		}
	}
}
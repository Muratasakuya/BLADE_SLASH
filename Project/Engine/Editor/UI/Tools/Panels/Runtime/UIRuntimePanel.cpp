#include "UIRuntimePanel.h"

using namespace SakuEngine;

//============================================================================
//	UIRuntimePanel classMethods
//============================================================================

void UIRuntimePanel::ImGui(UIToolContext& context) {

	UIAsset* asset = context.GetSelectedAsset();
	if (!asset) {
		ImGui::TextWrapped("No UI Asset selected.");
		return;
	}

	uint32_t keepSelectedUid = 0;
	if (ImGui::Checkbox("Activate", &asset->isActive)) {

		// 非アクティブ化する場合、選択中要素のUIDを保持しておく
		if (!asset->isActive) {
			if (context.selectedElement.IsValid() && asset->elements.IsAlive(context.selectedElement)) {
				if (auto* element = asset->Get(context.selectedElement)) {

					keepSelectedUid = element->uid;
				}
			}
		}

		// アクティブ、非アクティブ化を実行
		asset->SetActivate(asset->isActive);

		// 非アクティブ化した場合、選択中要素をUIDから復元する
		if (!asset->isActive && keepSelectedUid != 0) {

			UIElement::Handle found{};
			asset->elements.ForEachAlive([&](UIElement::Handle handle, const UIElement& element) {
				// UIDが一致したら見つけたハンドルを保存
				if (element.uid == keepSelectedUid) {
					found = handle;
				}
				});
			// 見つかったら選択中要素を復元、見つからなかったらルート要素を選択
			context.selectedElement = asset->elements.IsAlive(found) ? found : asset->rootHandle;
		}
	}
}
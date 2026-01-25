#include "UIVisualDesignerPanel.h"

using namespace SakuEngine;

//============================================================================
//	UIAssetPanel classMethods
//============================================================================

void UIVisualDesignerPanel::ImGui(UIToolContext& context) {

	context;

	// 描画結果表示
	ImGui::Image(ImTextureID(textureGPUHandle_.ptr), gameViewSize_);
}
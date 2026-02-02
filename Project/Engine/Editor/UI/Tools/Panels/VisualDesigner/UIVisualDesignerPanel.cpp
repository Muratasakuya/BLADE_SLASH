#include "UIVisualDesignerPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	UIAssetPanel classMethods
//============================================================================

void UIVisualDesignerPanel::ImGui(UIToolContext& context) {

	context;

	// 描画結果表示
	ImGui::Image(ImTextureID(textureGPUHandle_.ptr), gameViewSize_);

	// 入力エリア設定
	Input* input = SakuEngine::Input::GetInstance();
	Vector2 min = Vector2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);
	Vector2 size = Vector2(ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y);
	input->SetViewRect(InputViewArea::UIEditor, min, size);
}
#include "UICoreAssetPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetEditor.h>

//============================================================================
//	UICoreAssetPanel classMethods
//============================================================================

void UICoreAssetPanel::ImGui([[maybe_unused]] UIToolContext& context) {

	ImGui::PushID("UICoreAssetPanel");

	AssetEditor::GetInstance()->ImGui();

	ImGui::PopID();
}
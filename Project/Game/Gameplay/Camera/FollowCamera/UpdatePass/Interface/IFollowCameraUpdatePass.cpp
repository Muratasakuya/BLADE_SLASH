#include "IFollowCameraUpdatePass.h"

//============================================================================
//	IFollowCameraUpdatePass classMethods
//============================================================================

void IFollowCameraUpdatePass::ImGuiCommon() {

	ImGui::Checkbox("Enable", &enable_);
}
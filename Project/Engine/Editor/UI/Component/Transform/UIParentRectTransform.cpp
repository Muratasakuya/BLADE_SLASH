#include "UIParentRectTransform.h"

using namespace SakuEngine;

//============================================================================
//	UIParentRectTransform classMethods
//============================================================================

void UIParentRectTransform::ImGui(const ImVec2& itemSize) {

	transform.ImGuiCommon(itemSize.x);
}
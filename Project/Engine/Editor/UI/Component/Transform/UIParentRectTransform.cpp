#include "UIParentRectTransform.h"

using namespace SakuEngine;

//============================================================================
//	UIParentRectTransform classMethods
//============================================================================

void UIParentRectTransform::ImGui(const ImVec2& itemSize) {

	transform.ImGuiCommon(itemSize.x);
}

void UIParentRectTransform::FromJson(const Json& data) {

	transform.FromJsonCommon(data);
}

void UIParentRectTransform::ToJson(Json& data) {

	transform.ToJsonCommon(data);
}
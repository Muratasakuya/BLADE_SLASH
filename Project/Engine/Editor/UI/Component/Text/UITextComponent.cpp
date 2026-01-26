#include "UITextComponent.h"

using namespace SakuEngine;

//============================================================================
//	UITextComponent classMethods
//============================================================================

void UITextComponent::ImGui(const ImVec2& itemSize) {

	if (!text) {
		return;
	}
	text->ImGui(itemSize.x);
}
#include "UITextTransformComponent.h"

using namespace SakuEngine;

//============================================================================
//	UITextTransformComponent classMethods
//============================================================================

void UITextTransformComponent::ImGui(const ImVec2& itemSize) {

	if (!transform) {
		return;
	}
	transform->ImGui(itemSize.x);
}
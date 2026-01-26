#include "UISpriteComponent.h"

using namespace SakuEngine;

//============================================================================
//	UISpriteComponent classMethods
//============================================================================

void UISpriteComponent::ImGui(const ImVec2& itemSize) {

	if (!sprite) {
		return;
	}
	sprite->ImGui(itemSize.x);
}
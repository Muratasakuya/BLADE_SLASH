#include "UISpriteTransformComponent.h"

using namespace SakuEngine;

//============================================================================
//	UISpriteTransformComponent classMethods
//============================================================================

void UISpriteTransformComponent::ImGui(const ImVec2& itemSize) {

	if (!transform) {
		return;
	}
	transform->ImGui(itemSize.x);
}
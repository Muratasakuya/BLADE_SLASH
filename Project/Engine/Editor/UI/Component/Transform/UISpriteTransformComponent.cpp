#include "UISpriteTransformComponent.h"

using namespace SakuEngine;

//============================================================================
//	UISpriteTransformComponent classMethods
//============================================================================

void UISpriteTransformComponent::ImGui(const ImVec2& itemSize) {

	transform.ImGui(itemSize.x);
}
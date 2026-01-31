#include "UISelectableComponent.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UISelectableComponent classMethods
//============================================================================

void UISelectableComponent::ImGui([[maybe_unused]] const ImVec2& itemSize) {

	ImGui::Text(std::format("interactable: {}", interactable).c_str());
	ImGui::Text(std::format("wantVisible:  {}", wantVisible).c_str());
	ImGui::Text(std::format("state:        {}", EnumAdapter<UIElementState>::ToString(state)).c_str());

	EnumAdapter<UINavigationMode>::Combo("navigationMode", &navigation.mode);
	ImGui::SliderFloat("minDot", &navigation.minDot, 0.0f, 1.0f);

	if (navigation.mode == UINavigationMode::Explicit) {

		ImGui::InputScalar("up", ImGuiDataType_U32, &navigation.explicitNavi.up);
		ImGui::InputScalar("down", ImGuiDataType_U32, &navigation.explicitNavi.down);
		ImGui::InputScalar("left", ImGuiDataType_U32, &navigation.explicitNavi.left);
		ImGui::InputScalar("right", ImGuiDataType_U32, &navigation.explicitNavi.right);
	}
}

void UISelectableComponent::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	interactable = data.value("interactable", true);
	wantVisible = data.value("wantVisible", true);

	navigation.mode = EnumAdapter<UINavigationMode>::FromString(data.value("navigationMode", "Automatic")).value();
	navigation.minDot = data.value("minDot", 0.24f);

	const Json& explicitData = data["explicit"];
	navigation.explicitNavi.up = explicitData.value("up", 0);
	navigation.explicitNavi.down = explicitData.value("down", 0);
	navigation.explicitNavi.left = explicitData.value("left", 0);
	navigation.explicitNavi.right = explicitData.value("right", 0u);
}

void UISelectableComponent::ToJson(Json& data) {

	data["interactable"] = interactable;
	data["wantVisible"] = wantVisible;

	data["navigationMode"] = EnumAdapter<UINavigationMode>::ToString(navigation.mode);
	data["minDot"] = navigation.minDot;

	Json explicitData;
	explicitData["up"] = navigation.explicitNavi.up;
	explicitData["down"] = navigation.explicitNavi.down;
	explicitData["left"] = navigation.explicitNavi.left;
	explicitData["right"] = navigation.explicitNavi.right;
	data["explicit"] = explicitData;
}
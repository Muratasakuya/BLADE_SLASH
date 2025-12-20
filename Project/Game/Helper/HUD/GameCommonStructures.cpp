#include "GameCommonStructures.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>

// imgui
#include <imgui.h>

//============================================================================
//	GameCommonStructures classMethods
//============================================================================

bool GameCommon::HUDInitParameter::ImGui(const std::string& label) {

	bool edit = false;

	ImGui::SeparatorText(label.c_str());

	edit = ImGui::DragFloat2(("translation##" + label).c_str(), &translation.x, 1.0f);

	return edit;
}

void GameCommon::HUDInitParameter::ApplyJson(const Json& data) {

	translation = translation.FromJson(data["translation"]);
}

void GameCommon::HUDInitParameter::SaveJson(Json& data) {

	data["translation"] = translation.ToJson();
}

void GameCommon::SetInitParameter(SakuEngine::GameObject2D& sprite, const  GameCommon::HUDInitParameter& parameter) {

	sprite.SetTranslation(parameter.translation);
}
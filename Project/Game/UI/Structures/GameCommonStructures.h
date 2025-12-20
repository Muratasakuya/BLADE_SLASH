#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector2.h>

// c++
#include <string>
// front
namespace SakuEngine {
	class GameObject2D;
}

//============================================================================
//	GameCommonStructures class
//============================================================================

namespace GameCommon {

	// HUDの初期化値
	struct HUDInitParameter {

		SakuEngine::Vector2 translation; // 座標

		// imgui
		bool ImGui(const std::string& label);

		// json
		void ApplyJson(const Json& data);
		void SaveJson(Json& data);
	};

	void SetInitParameter(SakuEngine::GameObject2D& sprite, const  GameCommon::HUDInitParameter& parameter);
}
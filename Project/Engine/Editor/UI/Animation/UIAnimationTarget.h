#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cstdint>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationTarget structures
	//============================================================================
	
	// UIアニメーションターゲットタイプ
	enum class UIAnimationTargetType :
		uint32_t {

		ParentRectTransform,
		SpriteTransform,
		TextTransform,
		SpriteMaterial,
		TextMaterial,
	};
	// UIアニメーションプロパティ
	enum class UIAnimationProperty :
		uint32_t {

		Translation, // Vector2
		Scale,       // Vector2
		Rotation,    // float
		Color,       // Color
	};
} // SakuEngine
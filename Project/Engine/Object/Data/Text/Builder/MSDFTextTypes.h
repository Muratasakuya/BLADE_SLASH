#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector2.h>

namespace SakuEngine {

	// テキストの測定結果
	struct MSDFTextBounds {

		Vector2 min{};
		Vector2 max{};

		float Width() const { return max.x - min.x; }
		float Height() const { return max.y - min.y; }
	};

	// 文字間隔モード
	enum class TextSpacingMode :
		uint8_t {

		Tight,
		FixedAdvance
	};
	struct TextSpacingParams {

		TextSpacingMode mode = TextSpacingMode::Tight;
		bool applyOnlyToNumericSet = false; // trueなら数字/記号だけ固定幅

		float fixedAdvance = 0.0f; // 0なら自動算出
		float cellAlign = 0.0f;    // 0:左, 0.5:中央, 1:右
	};

} // SakuEngine
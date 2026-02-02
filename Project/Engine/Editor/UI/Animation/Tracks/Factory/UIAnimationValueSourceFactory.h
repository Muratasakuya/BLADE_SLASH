#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/ValueSource/Interface/IValueSource.h>
#include <Engine/Utility/Animation/ValueSource/LerpValueSource.h>

// c++
#include <memory>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationValueSourceFactory class
	//	UIアニメーションの値ソース生成を行うクラス
	//============================================================================

	template <typename T>
	std::unique_ptr<IValueSource<T>> CreateValueSource(const std::string& typeName) {

		// 線形補間
		if (typeName == "Lerp") {

			return std::make_unique<LerpValueSource<T>>();
		}
		return nullptr;
	}
} // SakuEngine
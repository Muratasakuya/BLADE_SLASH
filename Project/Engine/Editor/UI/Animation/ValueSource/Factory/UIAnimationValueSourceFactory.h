#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/UIAnimationClip.h>
#include <Engine/Editor/UI/Animation/ValueSource/UILerpValueSource.h>
#include <Engine/Utility/Animation/ValueSource/Interface/IValueSource.h>

// c++
#include <memory>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationValueSourceFactory class
	//	UIアニメーションの値ソース生成を行うクラス
	//============================================================================

	template <typename T>
	std::unique_ptr<IValueSource<T>> CreateValueSource(UIAnimationType type) {

		switch (type) {
		case UIAnimationType::Lerp:
			
			return std::make_unique<UILerpValueSource<T>>();
		}
		return nullptr;
	}
} // SakuEngine
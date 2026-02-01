#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Tracks/Interface/IUIAnimationTrackRuntime.h>

// c++
#include <memory>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationTrackFactory class
	//	UIアニメーショントラックの生成を行うクラス
	//============================================================================
	class UIAnimationTrackFactory {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIAnimationTrackFactory() = default;
		~UIAnimationTrackFactory() = default;

		static std::unique_ptr<IUIAnimationTrackRuntime> Create(const UIAnimationTrackDefinition& definition);
	};
} // SakuEngine
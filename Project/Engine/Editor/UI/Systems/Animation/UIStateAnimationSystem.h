#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/Interface/IUISystem.h>

namespace SakuEngine {

	//============================================================================
	//	UIStateAnimationSystem class
	//	UIの状態に合わせてアニメーションを再生するシステム
	//============================================================================
	class UIStateAnimationSystem :
		public IUISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIStateAnimationSystem() = default;
		~UIStateAnimationSystem() = default;

		void Update(UISystemContext* context, UIAsset& asset) override;
	};
} // SakuEngine
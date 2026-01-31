#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/System/Interface/IUISystem.h>

namespace SakuEngine {

	//============================================================================
	//	UIInputNavigationSystem class
	//	UI入力ナビゲーションシステム
	//============================================================================
	class UIInputNavigationSystem :
		public IUISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIInputNavigationSystem() = default;
		~UIInputNavigationSystem() = default;

		void Update(UIAsset& asset) override;
	};
} // SakuEngine
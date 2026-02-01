#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/Interface/IUISystem.h>

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

		void Update(UISystemContext* context, UIAsset& asset) override;
	};
} // SakuEngine
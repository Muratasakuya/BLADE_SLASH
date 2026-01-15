#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Base/UISlateWidget.h>

namespace SakuEngine {

	//============================================================================
	//	UISlateLeafWidget class
	//	
	//============================================================================
	class UISlateLeafWidget :
		public UISlateWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISlateLeafWidget() = default;
		~UISlateLeafWidget() = default;

		// 子供を持つか
		bool IsPanel() const override { return false; }
	};
} // SakuEngine
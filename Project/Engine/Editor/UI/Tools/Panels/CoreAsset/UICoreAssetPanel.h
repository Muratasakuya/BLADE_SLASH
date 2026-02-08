#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>

namespace SakuEngine {

	//============================================================================
	//	UICoreAssetPanel class
	//	UIに使用するコアアセットを提供するパネル
	//============================================================================
	class UICoreAssetPanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UICoreAssetPanel() = default;
		~UICoreAssetPanel() = default;

		// エディター
		void ImGui(UIToolContext& context) override;

		//--------- accessor -----------------------------------------------------

		// エディタの名前、パネルに表示する
		const char* GetName() const override { return "UI CoreAsset"; }
	};
} // SakuEngine
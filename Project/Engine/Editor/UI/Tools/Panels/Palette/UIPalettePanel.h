#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>
#include <Engine/Editor/UI/Tools/Registry/UIPaletteRegistry.h>

namespace SakuEngine {

	//============================================================================
	//	UIPalettePanel class
	//	UI要素のパレットを表示、選択するパネル
	//============================================================================
	class UIPalettePanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIPalettePanel() = default;
		~UIPalettePanel() = default;

		// エディター
		void ImGui(UIToolContext& context) override;

		//--------- accessor -----------------------------------------------------

		// エディタの名前、パネルに表示する
		const char* GetName() const override { return "Palette"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// カテゴリを描画
		void DrawCategory(UIToolContext& context, UIPaletteItemCategory category);
	};
} // SakuEngine
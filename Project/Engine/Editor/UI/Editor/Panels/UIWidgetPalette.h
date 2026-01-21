#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Panels/Interface/UIWidgetEditorPanel.h>

// c++
#include <string>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetPalette class
	//	UIウィジェットのパレットを表示、提供するクラス
	//============================================================================
	class UIWidgetPalette :
		public IUIWidgetEditorPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetPalette() = default;
		~UIWidgetPalette() = default;

		// エディター
		void Draw(UIWidgetEditorContext& context) override;

		//--------- accessor -----------------------------------------------------

		// パネルの名前
		const char* GetPanelName() const override { return "Palette"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		char searchBuf_[256]{};
	};
} // SakuEngine
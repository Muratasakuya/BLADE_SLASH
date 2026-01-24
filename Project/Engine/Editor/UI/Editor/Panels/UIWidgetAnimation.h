#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Panels/Interface/UIWidgetEditorPanel.h>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetAnimation class
	//	UIウィジェットのアニメーションを作成、編集して提供するクラス
	//============================================================================
	class UIWidgetAnimation :
		public IUIWidgetEditorPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetAnimation() = default;
		~UIWidgetAnimation() = default;

		// エディター
		void Draw(UIWidgetEditorContext& context) override;

		//--------- accessor -----------------------------------------------------

		// パネルの名前
		const char* GetPanelName() const override { return "Animation"; }
	};
} // SakuEngine
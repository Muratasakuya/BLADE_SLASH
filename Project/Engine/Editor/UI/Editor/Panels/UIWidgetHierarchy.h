#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Panels/Interface/UIWidgetEditorPanel.h>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetHierarchy class
	//	UIウィジェットのヒエラルキーを表示、編集するクラス
	//============================================================================
	class UIWidgetHierarchy :
		public IUIWidgetEditorPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetHierarchy() = default;
		~UIWidgetHierarchy() = default;

		// エディター
		void Draw(UIWidgetEditorContext& context) override;

		//--------- accessor -----------------------------------------------------

		// パネルの名前
		const char* GetPanelName() const override { return "UIHierarchy"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		void DrawNode(UIWidgetEditorContext& ctx, uint32_t nodeId);
	};
} // SakuEngine
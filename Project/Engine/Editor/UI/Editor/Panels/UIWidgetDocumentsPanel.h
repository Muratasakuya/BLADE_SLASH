#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Panels/Interface/UIWidgetEditorPanel.h>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetDocumentsPanel class
	//	
	//============================================================================
	class UIWidgetDocumentsPanel :
		public IUIWidgetEditorPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetDocumentsPanel() = default;
		~UIWidgetDocumentsPanel() = default;

		// エディター
		void Draw(UIWidgetEditorContext& context) override;

		//--------- accessor -----------------------------------------------------

		// パネルの名前
		const char* GetPanelName() const override { return "UIWidgetEditor"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 新規ドキュメントキー入力用バッファ
		char newKey_[128] = "NewWidget";
	};
} // SakuEngine
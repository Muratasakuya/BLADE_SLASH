#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cstdint>
#include <string>

namespace SakuEngine {

	// front
	class UIWidgetEditorContext;

	//============================================================================
	//	IUIWidgetEditorPanel  class
	//	UIウィジェットのエディターパネルのインターフェース
	//============================================================================
	class IUIWidgetEditorPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IUIWidgetEditorPanel() = default;
		virtual ~IUIWidgetEditorPanel() = default;

		// エディター
		virtual void Draw(UIWidgetEditorContext& context) = 0;

		// ドキュメントが変更されたときの通知
		virtual void OnDocumentChanged([[maybe_unused]] UIWidgetEditorContext& context) {}

		//--------- accessor -----------------------------------------------------

		// パネルの名前
		virtual const char* GetPanelName() const = 0;
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Panels/Interface/UIWidgetEditorPanel.h>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetDetail class
	//	UIウィジェットの詳細の表示、編集を行うクラス
	//============================================================================
	class UIWidgetDetail :
		public IUIWidgetEditorPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetDetail() = default;
		~UIWidgetDetail() = default;

		// エディター
		void Draw(UIWidgetEditorContext& context) override;

		//--------- accessor -----------------------------------------------------

		// パネルの名前
		const char* GetPanelName() const override { return "Details"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		// 共通部分の描画
		void DrawCommon(UIWidgetEditorContext& context, uint32_t id);
		// 型固有部分の描画
		void DrawTypeSpecific(UIWidgetEditorContext& context, uint32_t id, const std::string& type);

		// Vector4でのマージン編集
		static bool EditMargin(const char* label, float& left, float& top, float& right, float& bottom);
	};
} // SakuEngine
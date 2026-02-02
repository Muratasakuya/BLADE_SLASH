#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>

namespace SakuEngine {

	//============================================================================
	//	UIRuntimePanel class
	//	UIのランタイム情報、実行時情報を表示、編集するパネル
	//============================================================================
	class UIRuntimePanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIRuntimePanel() = default;
		~UIRuntimePanel() = default;

		// エディター
		void ImGui(UIToolContext& context) override;

		//--------- accessor -----------------------------------------------------

		// エディタの名前、パネルに表示する
		const char* GetName() const override { return "Runtime##UIRuntimePanel"; }
	};
} // SakuEngine
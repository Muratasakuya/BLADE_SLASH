#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

namespace SakuEngine {

	//============================================================================
	//	UIDetailPanel class
	//	UI要素の詳細情報を表示、編集するパネル
	//============================================================================
	class UIDetailPanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIDetailPanel() = default;
		~UIDetailPanel() = default;

		// エディター
		void ImGui(UIToolContext& context) override;

		//--------- accessor -----------------------------------------------------

		// エディタの名前、パネルに表示する
		const char* GetName() const override { return "Detail"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// コンポーネント名入力用
		InputImGui inputText_;
		// jsonの保存状態
		JsonSaveState jsonSaveElementState_;
		JsonSaveState jsonSaveComponentState_;
	};
} // SakuEngine
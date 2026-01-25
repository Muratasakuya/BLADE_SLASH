#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

namespace SakuEngine {

	//============================================================================
	//	UIAssetPanel class
	//	UIアセットを表示、編集するパネル
	//============================================================================
	class UIAssetPanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIAssetPanel() = default;
		~UIAssetPanel() = default;

		// エディター
		void ImGui(UIToolContext& context) override;

		//--------- accessor -----------------------------------------------------

		// エディタの名前、パネルに表示する
		const char* GetName() const override { return "UI Asset"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// UIアセット名入力用
		InputImGui inputText_;

		//--------- functions ----------------------------------------------------

	};
} // SakuEngine
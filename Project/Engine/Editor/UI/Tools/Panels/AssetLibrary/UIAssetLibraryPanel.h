#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

namespace SakuEngine {

	//============================================================================
	//	UIAssetLibraryPanel class
	//	UIアセットを表示、編集するパネル
	//============================================================================
	class UIAssetLibraryPanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIAssetLibraryPanel() = default;
		~UIAssetLibraryPanel() = default;

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
		// jsonの保存状態
		JsonSaveState jsonSaveState_;
	};
} // SakuEngine
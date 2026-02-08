#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>
#include <Engine/Editor/UI/Animation/UIAnimationClip.h>
#include <Engine/Object/Data/Canvas/CanvasCommon.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationPanel class
	//	UIアニメーションを表示、編集するパネル
	//============================================================================
	class UIAnimationPanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIAnimationPanel() = default;
		~UIAnimationPanel() = default;

		// エディター
		void ImGui(UIToolContext& context) override;

		//--------- accessor -----------------------------------------------------

		// エディタの名前、パネルに表示する
		const char* GetName() const override { return "Animation"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 新規作成用
		InputImGui newName_;
		CanvasType newCanvasType_ = CanvasType::Sprite;

		// 選択中
		uint32_t selectedUid_ = 0;
		uint32_t lastSelectedUid_ = 0;

		// リネーム用
		InputImGui rename_;
	};
} // SakuEngine
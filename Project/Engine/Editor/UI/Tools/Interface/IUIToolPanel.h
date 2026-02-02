#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/UIToolContext.h>

namespace SakuEngine {

	//============================================================================
	//	IUIToolPanel class
	//	UIツールパネルのインターフェース
	//============================================================================
	class IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IUIToolPanel() = default;
		virtual ~IUIToolPanel() = default;

		// エディター
		virtual void ImGui(UIToolContext& context) = 0;

		//--------- variables ----------------------------------------------------

		// ペイロードID(UIパレットアイテムタイプ)
		static constexpr const char* kPalettePayloadId = "UI_PALETTE_ITEM_TYPE";
		static constexpr const char* kHierarchyPayloadId = "UI_HIERARCHY_ELEMENT_HANDLE";
		static constexpr const char* kAnimationPayloadId = "UI_ANIMATION_UID";

		//--------- accessor -----------------------------------------------------

		// エディタの名前、パネルに表示する
		virtual const char* GetName() const = 0;
	};
} // SakuEngine
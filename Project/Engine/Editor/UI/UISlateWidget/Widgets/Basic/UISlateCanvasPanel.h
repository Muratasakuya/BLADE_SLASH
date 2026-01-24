#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Widgets/UISlatePanelWidget.h>
#include <Engine/Editor/UI/UISlateWidget/Mehtods/UIWidgetFactory.h>

namespace SakuEngine {

	//============================================================================
	//	UISlateCanvasPanel class
	//	
	//============================================================================
	class UISlateCanvasPanel :
		public UISlatePanelWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISlateCanvasPanel() = default;
		~UISlateCanvasPanel() = default;

		// 子の配置
		void ArrangeChildren(const UIRect& allotted, std::vector<UIArrangedWidget>& outArranged) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- accessor -----------------------------------------------------

		std::string GetTypeName() const override { return "CanvasPanel"; }
	};

	// 自動登録
	static inline UIWidgetAutoRegister<UISlateCanvasPanel> AutoRegistryCanvasPanel("CanvasPanel");
} // SakuEngine
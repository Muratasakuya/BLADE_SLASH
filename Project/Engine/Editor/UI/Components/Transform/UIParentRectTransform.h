#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/Interface/IUIComponent.h>
#include <Engine/Object/Data/Transform/Transform.h>

namespace SakuEngine {

	//============================================================================
	//	UIParentRectTransform class
	//	UI親要素のトランスフォームデータ
	//============================================================================
	class UIParentRectTransform :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIParentRectTransform() = default;
		~UIParentRectTransform() = default;

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- variables ----------------------------------------------------

		BaseTransform2D transform;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::ParentRectTransform; }
	};
} // SakuEngine
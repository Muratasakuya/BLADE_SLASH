#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Interface/IUIComponent.h>
#include <Engine/Object/Data/Transform/Transform.h>

namespace SakuEngine {

	//============================================================================
	//	UITextTransformComponent class
	//	UIのTransformコンポーネント
	//============================================================================
	class UITextTransformComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UITextTransformComponent() = default;
		~UITextTransformComponent() = default;

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- variables ----------------------------------------------------

		TextTransform2D* transform = nullptr;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::TextTransform; }
	};
} // SakuEngine
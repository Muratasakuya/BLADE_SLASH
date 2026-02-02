#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/Interface/IUIComponent.h>
#include <Engine/Object/Data/Material/Material.h>

namespace SakuEngine {

	//============================================================================
	//	UITextMaterialComponent class
	//	UIのマテリアルコンポーネント
	//============================================================================
	class UITextMaterialComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UITextMaterialComponent() = default;
		~UITextMaterialComponent() = default;

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- variables ----------------------------------------------------

		MSDFTextMaterial* material = nullptr;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::TextMaterial; }
	};
} // SakuEngine
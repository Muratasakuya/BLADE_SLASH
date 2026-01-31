#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Interface/IUIComponent.h>
#include <Engine/Object/Data/Material/Material.h>

namespace SakuEngine {

	//============================================================================
	//	UISpriteMaterialComponent class
	//	UIのマテリアルコンポーネント
	//============================================================================
	class UISpriteMaterialComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISpriteMaterialComponent() = default;
		~UISpriteMaterialComponent() = default;

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- variables ----------------------------------------------------

		SpriteMaterial* material = nullptr;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::SpriteMaterial; }
	};
} // SakuEngine
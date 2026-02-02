#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/Interface/IUIComponent.h>
#include <Engine/Object/Data/Sprite/Sprite.h>

// c++
#include <string>

namespace SakuEngine {

	//============================================================================
	//	UISpriteComponent class
	//	UIのスプライトコンポーネント
	//============================================================================
	class UISpriteComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISpriteComponent() = default;
		~UISpriteComponent() = default;

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- variables ----------------------------------------------------

		// オブジェクトID
		uint32_t objectId = 0;

		Sprite* sprite = nullptr;

		// デフォルト
		// テクスチャ名
		std::string defaultTextureName = "white";

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::Sprite; }
	};
} // SakuEngine
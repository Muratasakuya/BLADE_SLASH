#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Interface/IUIComponent.h>
#include <Engine/Object/Data/Text/MSDFText.h>

// c++
#include <string>

namespace SakuEngine {

	//============================================================================
	//	UITextComponent class
	//	UIのテキストコンポーネント
	//============================================================================
	class UITextComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UITextComponent() = default;
		~UITextComponent() = default;

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- variables ----------------------------------------------------

		// オブジェクトID
		uint32_t objectId = 0;

		MSDFText* text = nullptr;

		// デフォルト
		// アトラステクスチャ名
		std::string defaultAtlasTextureName = "DS-DIGIB_msdf";
		// フォントパス
		std::string defaultFontPath = "Assets/Json/Atlas/DS-DIGIB_msdf.json";
		// テキスト内容
		std::string defaultText = "Text_ABC";

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::Text; }
	};
} // SakuEngine
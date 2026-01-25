#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Interface/IUIComponent.h>

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

		//--------- variables ----------------------------------------------------

		// オブジェクトID
		uint32_t objectId = 0;

		// アトラステクスチャ名
		std::string atlasTextureName;
		// フォントパス
		std::string fontPath;
		// テキスト内容
		std::string text;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::Text; }
	};
} // SakuEngine
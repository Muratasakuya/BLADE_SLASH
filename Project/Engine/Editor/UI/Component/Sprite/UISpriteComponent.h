#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Interface/IUIComponent.h>

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

		//--------- variables ----------------------------------------------------

		// オブジェクトID
		uint32_t objectId = 0;

		// テクスチャ名
		std::string textureName;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::Sprite; }
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cstdint>
// imgui
#include <imgui.h>

namespace SakuEngine {

	//============================================================================
	//	IUIComponent structures
	//============================================================================

	// UIコンポーネントの種類
	enum class UIComponentType :
		uint32_t {

		ParentRectTransform,
		SpriteTransform,
		TextTransform,
		Sprite,
		Text,
	};

	//============================================================================
	//	IUIComponent class
	//	UIコンポーネントのインターフェース
	//============================================================================
	class IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IUIComponent() = default;
		virtual ~IUIComponent() = default;

		// エディター
		virtual void ImGui([[maybe_unused]] const ImVec2& itemSize) {}

		//--------- accessor -----------------------------------------------------

		// コンポーネントの種類を取得
		virtual UIComponentType GetType() const = 0;
	};
} // SakuEngine
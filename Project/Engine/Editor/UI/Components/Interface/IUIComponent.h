#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>

// c++
#include <cstdint>
#include <optional>
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
		SpriteMaterial,
		TextMaterial,
		Sprite,
		Text,
		Selectable,
		InputNavigation,
		StateAnimation,
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

		// json
		virtual void FromJson([[maybe_unused]] const Json& data) {}
		virtual void ToJson([[maybe_unused]] Json& data) {}

		//--------- accessor -----------------------------------------------------

		// jsonキャッシュの有無
		bool HasJsonCache() const { return jsonCache_.has_value(); }
		// jsonキャッシュの取得
		const Json& GetJsonCache() const { return jsonCache_.value(); }
		// jsonキャッシュのクリア
		void ClearJsonCache() { jsonCache_ = std::nullopt; }

		// コンポーネントの種類を取得
		virtual UIComponentType GetType() const = 0;
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// jsonデータのキャッシュ
		std::optional<Json> jsonCache_;
	};
} // SakuEngine
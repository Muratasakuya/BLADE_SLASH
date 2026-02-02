#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/Interface/IUIComponent.h>

// c++
#include <cstdint>

namespace SakuEngine {

	//============================================================================
	//	UISelectableComponent structures
	//============================================================================

	// 選択ナビゲート方法
	enum class UINavigationMode :
		uint8_t {

		None,      // ナビゲートしない
		Automatic, // 自動で判断する
		Explicit,  // 明示的に設定する
	};

	// 状態
	enum class UIElementState :
		uint8_t {

		Hidden, // 非表示
		ShowBegin, // 表示を開始する
		ShowEnd,   // 表示を終了する
		Showing,   // 表示中
		Focused,   // フォーカス中
		Decided,   // 決定
	};

	// 明示ナビゲーション設定
	struct UIExplicitNavigation {

		uint32_t up = 0;
		uint32_t down = 0;
		uint32_t left = 0;
		uint32_t right = 0;
	};

	// ナビゲーション設定
	struct UINavigationSettings {

		UINavigationMode mode = UINavigationMode::Automatic;
		UIExplicitNavigation explicitNavi{};

		// Automaticで自動でナビゲートする際の許容角度
		float minDot = 0.24f; // 0に近いほど斜めを許容、1に近いほど真っ直ぐのみ
	};

	//============================================================================
	//	UISelectableComponent class
	//	UI要素選択コンポーネント
	//============================================================================
	class UISelectableComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISelectableComponent() = default;
		~UISelectableComponent() = default;

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- variables ----------------------------------------------------

		 // 有効、無効設定
		bool interactable = true;
		// 表示ターゲット
		bool wantVisible = true;

		// 現在の状態
		UIElementState state = UIElementState::Showing;

		// ナビゲーション設定
		UINavigationSettings navigation{};

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::Selectable; }
	};
} // SakuEngine
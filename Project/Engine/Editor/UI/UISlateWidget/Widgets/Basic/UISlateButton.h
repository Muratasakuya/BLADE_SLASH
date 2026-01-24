#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Widgets/UISlatePanelWidget.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateImage.h>

// c++
#include <functional>

namespace SakuEngine {

	// UIボタン状態
	enum class UIEventButtonState :
		uint8_t {

		Normal,   // 通常状態
		Hovered,  // ホバー中
		Pressed,  // 押下中
		Disabled, // 非アクティブ状態
	};
	// UIボタンスタイル
	struct UIButtonStyle {

		// 各状態のテクスチャ名
		std::string normalTexture{};
		std::string hoveredTexture{};
		std::string pressedTexture{};
		std::string disabledTexture{};
	};

	//============================================================================
	//	UISlateButton class
	//	
	//============================================================================
	class UISlateButton :
		public UISlatePanelWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISlateButton() = default;
		~UISlateButton() = default;

		// ツリーに入った後に呼ばれる
		void OnAddedToTree(UIUserWidget& owner) override;
		// ポインターイベント処理の呼び出し
		UIReply OnPointerEvent(UIUserWidget& owner, const UIPointerEvent& event) override;

		// 描画パラメータを更新する
		void PaintDrawData(UIUserWidget& owner) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		// キーボード操作対応
		bool SupportsKeyboardFocus() const override { return true; }

		//--------- accessor -----------------------------------------------------

		// スタイル設定
		void SetButtonStyle(const UIButtonStyle& style) { style_ = style; }
		// クリック時コールバック設定
		void SetOnClicked(const std::function<void()>& onClicked) { onClicked_ = onClicked; }
		// クリックアクションキー設定
		void SetClickActionKey(const std::string& actionKey) { clickActionKey_ = actionKey; }

		// スタイル取得
		const UIButtonStyle& GetButtonStyle() const { return style_; }

		std::string GetTypeName() const override { return "Button"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 現在の状態
		UIEventButtonState state_ = UIEventButtonState::Normal;
		// スタイル
		UIButtonStyle style_{};

		// 入力時コールバック
		std::function<void()> onClicked_{};
		std::string clickActionKey_{};

		// 背景画像
		UISlateImage* background_ = nullptr;

		// 押下中か
		bool isPressedInside_ = false;
	};

	// 自動登録
	static inline UIWidgetAutoRegister<UISlateButton> AutoRegistryButton("Button");
} // SakuEngine
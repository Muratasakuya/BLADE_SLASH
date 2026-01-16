#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Widgets/UISlatePanelWidget.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateImage.h>

namespace SakuEngine {


	//============================================================================
	//	UISlateSlider class
	//	
	//============================================================================
	class UISlateSlider :
		public UISlatePanelWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISlateSlider() = default;
		~UISlateSlider() = default;

		// ツリーに入った後に呼ばれる
		void OnAddedToTree(UIUserWidget& owner) override;
		// プロパティ同期
		void SynchProperties(UIUserWidget& owner) override;
		// ポインターイベント処理の呼び出し
		UIReply OnPointerEvent(UIUserWidget& owner, const UIPointerEvent& event) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		// キーボード操作対応
		bool SupportsKeyboardFocus() const override { return true; }

		//--------- accessor -----------------------------------------------------

		// スライダー値設定(0.0f ~ 1.0f)
		void SetValue(float value) { value_ = std::clamp(value, 0.0f, 1.0f); }
		// バインディングキー設定
		void SetValueBindingKey(const std::string& bindingKey) { valueBindingKey_ = bindingKey; }
		// 値変更コールバック設定
		void SetOnValueChanged(const std::function<void(float)>& callback) { onValueChanged_ = callback; }

		// スライダー値取得(0.0f ~ 1.0f)
		float GetValue() const { return value_; }
		// バインディングキー取得
		const std::string& GetValueBindingKey() const { return valueBindingKey_; }

		std::string GetTypeName() const override { return "Slider"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// スライダー値(0.0f ~ 1.0f)
		float value_ = 0.0f;
		std::string valueBindingKey_{};

		// 値変更コールバック
		std::function<void(float)> onValueChanged_{};

		// 表示用イメージ
		UISlateImage* background_ = nullptr;
		UISlateImage* fill_ = nullptr;
		UISlateImage* knob_ = nullptr;

		// ドラッグ中か
		bool isDragging_ = false;

		//--------- functions ----------------------------------------------------

		// ポインタ位置から値を更新
		void UpdateFromPointer(const Vector2& screenPos);
	};

	// 自動登録
	static inline UIWidgetAutoRegister<UISlateSlider> AutoRegistrySlider("Slider");
} // SakuEngine
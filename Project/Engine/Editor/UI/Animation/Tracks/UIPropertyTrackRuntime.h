#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Tracks/Interface/IUIAnimationTrackRuntime.h>

namespace SakuEngine {

	//============================================================================
	//	UIPropertyTrackRuntime class
	//	UIプロパティトラックを実行時に扱うためのクラス
	//============================================================================
	template <typename T>
	class UIPropertyTrackRuntime :
		public IUIAnimationTrackRuntime {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		// UIプロパティアクセスの型定義
		using AccessBuilder = std::function<UIPropertyAccess<T>(UIAsset&, UIElement::Handle, CanvasType)>;

		UIPropertyTrackRuntime(UIAnimationDataType type, AnimationApplyMode applyMode,
			std::unique_ptr<IValueSource<T>> valueSource, AccessBuilder builder);
		~UIPropertyTrackRuntime() = default;

		// アニメーションの開始
		void Start(UIAsset& asset, const UIElement::Handle& elementHandle, CanvasType canvasType) override;
		// アニメーションの更新
		void Update(UIAsset& asset, const UIElement::Handle& elementHandle, CanvasType canvasType) override;

		//--------- accessor -----------------------------------------------------

		bool IsFinished() const override { return finished_; }

		UIAnimationDataType GetType() const override { return type_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 補間データタイプ
		UIAnimationDataType type_;
		// アニメーション適用モード
		AnimationApplyMode applyMode_;

		// 値ソース
		std::unique_ptr<IValueSource<T>> valueSource_;
		// アニメーション対象のUIプロパティアクセス
		UIPropertyAccess<T> propertyAccess_;
		AccessBuilder builder_;
		// 基準値
		T baseValue_{};

		// アニメーション状態
		bool active_ = false;
		bool finished_ = true;
	};

	//============================================================================
	//	UIPropertyTrackRuntime templateMethods
	//============================================================================

	template<typename T>
	inline UIPropertyTrackRuntime<T>::UIPropertyTrackRuntime(UIAnimationDataType type, AnimationApplyMode applyMode,
		std::unique_ptr<IValueSource<T>> valueSource, AccessBuilder builder) {

		type_ = type;
		applyMode_ = applyMode;
		valueSource_ = std::move(valueSource);
		builder_ = builder;
	}

	template<typename T>
	inline void UIPropertyTrackRuntime<T>::Start(UIAsset& asset, const UIElement::Handle& elementHandle, CanvasType canvasType) {

		// フラグをリセット
		finished_ = true;
		active_ = false;

		// 有効かチェック
		if (!valueSource_ || !builder_) {
			return;
		}
		propertyAccess_ = builder_(asset, elementHandle, canvasType);
		if (!propertyAccess_.Valid()) {
			return;
		}

		// 基準値を取得して値ソースを開始
		baseValue_ = propertyAccess_.getter();
		valueSource_->Start(baseValue_);
		// フラグをセット
		active_ = true;
		finished_ = false;
	}
	template<typename T>
	inline void UIPropertyTrackRuntime<T>::Update(UIAsset& /*asset*/,
		const UIElement::Handle& /*elementHandle*/, CanvasType /*canvasType*/) {

		// 無効状態では処理しない
		if (!active_ || !valueSource_ || !propertyAccess_.Valid()) {
			return;
		}

		// 値ソースを更新
		valueSource_->Update();

		// 適用モードに応じて値を調整
		propertyAccess_.setter(valueSource_->GetValue());

		// 終了判定
		if (valueSource_->IsFinished()) {

			active_ = false;
			finished_ = true;
		}
	}
} // SakuEngine
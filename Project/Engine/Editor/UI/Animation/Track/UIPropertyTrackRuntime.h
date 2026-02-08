#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Track/Interface/IUIAnimationTrackRuntime.h>
#include <Engine/Editor/UI/Animation/ValueSource/Interface/IUIValueSource.h>
#include <Engine/Utility/Animation/ValueSource/AnimationChannel.h>

// c++
#include <functional>

namespace SakuEngine {

	//============================================================================
	//	UIPropertyTrackRuntime class
	//	UIプロパティトラックランタイム
	//============================================================================
	template <typename T>
	class UIPropertyTrackRuntime :
		public IUIAnimationTrackRuntime {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		// UIプロパティアクセス構築関数型
		using BuildAccessFn = std::function<UIPropertyAccess<T>(UIAsset&, const UIElement::Handle&)>;

		UIPropertyTrackRuntime(AnimationApplyMode apply, std::unique_ptr<IUIValueSourceRuntime<T>> runtime,
			BuildAccessFn buildAccess) {

			// メンバ初期化
			applyMode_ = apply;
			runtime_ = std::move(runtime);
			buildAccess_ = buildAccess;
		}
		~UIPropertyTrackRuntime() = default;

		// アニメーションの開始
		void Start(UIAsset& asset, const UIElement::Handle& handle) override {

			// プロパティアクセスの構築
			access_ = buildAccess_(asset, handle);
			// アクセスが無効なら処理を抜ける
			if (!access_.Valid()) {
				return;
			}

			// 基準値の取得とランタイムの開始
			base_ = access_.getter();
			runtime_->Reset();
			runtime_->Start(base_);
		}

		// アニメーションの更新
		void Update([[maybe_unused]] UIAsset& asset, [[maybe_unused]] const UIElement::Handle& handle) override {

			// アクセスが無効なら処理を抜ける
			if (!access_.Valid()) {
				return;
			}
			// ランタイムの更新
			runtime_->Update();

			// 値の取得と適用
			T value = runtime_->GetValue();
			if (applyMode_ == AnimationApplyMode::Additive) {

				access_.setter(base_ + value);
			} else {

				access_.setter(value);
			}
		}

		//--------- accessor -----------------------------------------------------

		// アニメーションの終了判定
		bool IsFinished() const override { return runtime_->IsFinished(); }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 基準値
		T base_{};

		// アニメーション適用モード
		AnimationApplyMode applyMode_;
		// 値ソースランタイム
		std::unique_ptr<IUIValueSourceRuntime<T>> runtime_;

		// プロパティアクセス構築関数
		BuildAccessFn buildAccess_;
		// プロパティアクセス
		UIPropertyAccess<T> access_{};
	};
} // SakuEngine
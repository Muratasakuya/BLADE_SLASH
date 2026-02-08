#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/ValueSource/Interface/IUIValueSource.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

// c++
#include <memory>

namespace SakuEngine {

	//============================================================================
	//	UILerpValueSourceRuntime class
	//	UIアニメーション用の線形補間値ソース
	//============================================================================
	template <typename T>
	class UILerpValueSourceRuntime :
		public IUIValueSourceRuntime<T> {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UILerpValueSourceRuntime(const SimpleAnimation<T>& animationParam) {

			// アニメーションパラメータのコピー
			SimpleAnimation<T> param = animationParam;
			// Json経由でコピー
			Json data{};
			param.ToJson(data);
			animation_.FromJson(data);
		}
		~UILerpValueSourceRuntime() = default;

		void Start(const T& base) override {

			// 基準値をセット
			baseValue_ = base;
			currentValue_ = base;

			// アニメーション開始
			animation_.Start();
		}

		void Update() override {

			// 補間値の更新
			animation_.LerpValue(currentValue_);
		}

		void Reset() override {

			// リセット
			animation_.Reset();
		}

		//--------- accessor -----------------------------------------------------

		bool IsFinished() const override { return animation_.IsFinished(); }

		const T& GetValue() const override { return currentValue_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 値
		T currentValue_; // 現在の値
		T baseValue_;    // 基準値

		// 補間アニメーション
		SimpleAnimation<T> animation_;
	};

	//============================================================================
	//	UILerpValueSourceRuntime class
	//	UIアニメーション用の線形補間値ソースアセット
	//============================================================================
	template <typename T>
	class UILerpValueSourceAsset :
		public IUIValueSourceAsset<T> {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UILerpValueSourceAsset() = default;
		~UILerpValueSourceAsset() = default;

		std::unique_ptr<IUIValueSourceRuntime<T>> CreateRuntime() const override {

			return std::make_unique<UILerpValueSourceRuntime<T>>(animationParam_);
		}

		std::unique_ptr<IUIValueSourceAsset<T>> Clone() const override {

			// クローンの作成
			auto cloned = std::make_unique<UILerpValueSourceAsset<T>>();

			// アニメーションパラメータのコピー
			SimpleAnimation<T> param = animationParam_;
			// Json経由でコピー
			Json data{};
			param.ToJson(data);
			cloned->animationParam_.FromJson(data);
			return cloned;
		}

		void ImGui(const char* label) override {

			ImGui::PushID(label);

			animationParam_.ImGuiParam(label, true);
			animationParam_.ImGuiTimer(label);
			animationParam_.ImGuiLoop(label);

			ImGui::PopID();
		}

		void FromJson(const Json& data) override {

			animationParam_.FromJson(data);
		}

		void ToJson(Json& data) override {

			animationParam_.ToJson(data);
		}

		//--------- accessor -----------------------------------------------------

		UIValueSourceType GetType() const override { return UIValueSourceType::Lerp; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		SimpleAnimation<T> animationParam_{};
	};
} // SakuEngine
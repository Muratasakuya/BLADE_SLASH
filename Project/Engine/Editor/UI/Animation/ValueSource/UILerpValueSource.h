#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/ValueSource/Interface/IValueSource.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

namespace SakuEngine {

	//============================================================================
	//	UILerpValueSource class
	//	UIアニメーション用の線形補間値ソース
	//============================================================================
	template <typename T>
	class UILerpValueSource :
		public IValueSource<T> {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UILerpValueSource() = default;
		~UILerpValueSource() = default;

		// 更新開始
		void Start(const T& base) override;
		// 値の更新
		void Update() override;

		// リセット
		void Reset() override;

		// エディター
		void ImGui(const char* label) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- accessor -----------------------------------------------------

		// 終了判定
		bool IsFinished() const override { return animation_.IsFinished(); }

		// 値の取得
		T GetValue() const override { return currentValue_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 現在の値
		T currentValue_{};
		// 基準値
		T baseValue_{};

		// アニメーション
		SimpleAnimation<T> animation_{};
	};

	//============================================================================
	//	UILerpValueSource templateMethods
	//============================================================================

	template<typename T>
	inline void UILerpValueSource<T>::Start(const T& base) {

		// 補間開始
		baseValue_ = base;
		animation_.Start();
	}

	template<typename T>
	inline void UILerpValueSource<T>::Update() {

		// 値の補間更新
		animation_.LerpValue(currentValue_);
	}

	template<typename T>
	inline void UILerpValueSource<T>::Reset() {

		animation_.Reset();
	}

	template<typename T>
	inline void UILerpValueSource<T>::ImGui(const char* label) {

		const float itemWidth = 192.0f;

		ImGui::PushID(label);
		ImGui::PushItemWidth(itemWidth);

		ImGuiHelper::DragValue<T>("baseValue", baseValue_);

		ImGui::Spacing();
		ImGui::Separator();

		animation_.ImGuiParam(label, true);
		animation_.ImGuiTimer(label);
		animation_.ImGuiLoop(label);

		ImGui::PopItemWidth();
		ImGui::PopID();
	}

	template<typename T>
	inline void UILerpValueSource<T>::FromJson(const Json& data) {

		animation_.FromJson(data);
	}

	template<typename T>
	inline void UILerpValueSource<T>::ToJson(Json& data) {

		animation_.ToJson(data);
	}
} // SakuEngine
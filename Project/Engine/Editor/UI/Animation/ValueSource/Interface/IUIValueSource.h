#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>

// c++
#include <cstdint>
// imgui
#include <imgui.h>

namespace SakuEngine {

	//============================================================================
	//	UIValueSource structures
	//============================================================================

	// 値ソースタイプ
	enum class UIValueSourceType :
		uint32_t {

		Lerp,
	};

	//============================================================================
	//	UIValueSource class
	//	UIアニメーションの値ソース基底クラス
	//============================================================================
	template <typename T>
	class IUIValueSourceRuntime {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IUIValueSourceRuntime() = default;
		virtual ~IUIValueSourceRuntime() = default;

		// 開始
		virtual void Start(const T& base) = 0;
		// 更新
		virtual void Update() = 0;

		// リセット
		virtual void Reset() = 0;

		//--------- accessor -----------------------------------------------------

		// 処理が終了したか
		virtual bool IsFinished() const = 0;

		// 値を取得
		virtual const T& GetValue() const = 0;
	};

	//============================================================================
	//	IUIValueSourceAsset class
	//	UIアニメーションの値ソースアセット基底クラス
	//============================================================================
	template <typename T>
	class IUIValueSourceAsset {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IUIValueSourceAsset() = default;
		virtual ~IUIValueSourceAsset() = default;

		// 値ソースをランタイムで動かすためのインターフェースを生成
		virtual std::unique_ptr<IUIValueSourceRuntime<T>> CreateRuntime() const = 0;
		virtual std::unique_ptr<IUIValueSourceAsset<T>> Clone() const = 0;

		// エディター
		virtual void ImGui(const char* label) = 0;

		// json
		virtual void FromJson(const Json& data) = 0;
		virtual void ToJson(Json& data) = 0;

		//--------- accessor -----------------------------------------------------

		// 値ソースタイプを取得
		virtual UIValueSourceType GetType() const = 0;
	};
} // SakuEngine
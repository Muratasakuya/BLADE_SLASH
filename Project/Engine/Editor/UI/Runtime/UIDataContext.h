#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/AnyMoldEnum.h>

// c++
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetHandle struct
	//	UIウィジェットの参照ハンドル
	//============================================================================
	struct UIWidgetHandle {

		uint32_t id = 0;
		uint32_t generation = 0;

		// UI有効判定
		bool IsValid() const { return id != 0 && generation != 0; }
	};

	// 型制限
	template <typename T>
	concept UISupportedValueType =
		std::is_same_v<T, bool> ||
		std::is_same_v<T, int32_t> ||
		std::is_same_v<T, float> ||
		std::is_same_v<T, std::string>;

	//============================================================================
	//	UIDataContext class
	//	ゲームとUI間のデータバインディングコンテキスト
	//============================================================================
	class UIDataContext {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		//--------- structure -----------------------------------------------------

		// 任意の型、追加できるのはこれだけ
		using AnyValue = std::variant<bool, int32_t, float, std::string>;
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIDataContext() = default;
		~UIDataContext() = default;

		//--------- accessor -----------------------------------------------------

		// 変数設定
		template <UISupportedValueType T>
		void SetValue(const std::string& key, T value) { valueMap_[key] = value; }

		// 変数取得
		template <UISupportedValueType T>
		T GetValue(const std::string& key) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 変数マップ
		std::unordered_map<std::string, AnyValue> valueMap_{};
	};

	//============================================================================
	//	UIDataContext template Methods
	//============================================================================

	template<UISupportedValueType T>
	inline T UIDataContext::GetValue(const std::string& key) const {

		// 指定キーの値を取得
		auto it = valueMap_.find(key);
		if (it != valueMap_.end()) {
			// 型チェックして返す
			if (std::holds_alternative<T>(it->second)) {

				return std::get<T>(it->second);
			}
		}
		// 見つからなかった場合は型ごとのデフォルト値を返す
		return T{};
	}
} // SakuEngine
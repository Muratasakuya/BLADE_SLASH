#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <type_traits>
#include <cstddef>
#include <concepts>

namespace SakuEngine {

	//============================================================================
	//	DenseStateId
	//	enumに制限を行い、最後にCountを所持することを要求する
	//============================================================================
	template <class EnumT>
	concept DenseStateId = std::is_enum_v<EnumT> &&
		requires {
		EnumT::Count;
		{ static_cast<std::size_t>(EnumT::Count) } -> std::convertible_to<std::size_t>;
	};

	//============================================================================
	//	DenseStateToIndex
	//	enumの値をインデックスに変換する
	//============================================================================
	template <DenseStateId StateId>
	constexpr std::size_t DenseStateToIndex(StateId id) noexcept {

		return static_cast<std::size_t>(id);
	}
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Object/State/StateIdConcepts.h>

// c++
#include <array>
#include <memory>
#include <concepts>
#include <utility>

namespace SakuEngine {

	//============================================================================
	//	DenseStateStorage class
	//	状態の格納、管理を行う
	//============================================================================
	template <DenseStateId StateId, typename StateBase>
	class DenseStateStorage {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		DenseStateStorage() = default;
		~DenseStateStorage() = default;

		// 状態の生成
		template <typename StateT, typename... Args>
			requires std::derived_from<StateT, StateBase>&& std::constructible_from<StateT, Args...>
		StateT& Emplace(StateId id, Args&&... args) {

			auto index = SakuEngine::DenseStateToIndex(id);
			states_[index] = std::make_unique<StateT>(std::forward<Args>(args)...);
			return static_cast<StateT&>(*states_[index]);
		}

		// 全状態に対して関数を実行
		template <typename Fn>
		void ForEach(Fn&& function);

		//--------- accessor -----------------------------------------------------

		// 状態を所持しているか
		bool Has(StateId id) const;

		// 状態の取得
		StateBase& Get(StateId id);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 状態格納配列
		static constexpr size_t kCount = static_cast<size_t>(StateId::Count);
		std::array<std::unique_ptr<StateBase>, kCount> states_{};
	};

	//============================================================================
	//	DenseStateStorage templateMethods
	//============================================================================

	template<DenseStateId StateId, typename StateBase>
	template<typename Fn>
	inline void DenseStateStorage<StateId, StateBase>::ForEach(Fn&& function) {

		for (auto& state : states_) {
			if (state) {

				function(*state);
			}
		}
	}

	template<DenseStateId StateId, typename StateBase>
	inline bool DenseStateStorage<StateId, StateBase>::Has(StateId id) const {

		auto index = SakuEngine::DenseStateToIndex(id);
		return index < states_.size() && states_[index] != nullptr;
	}

	template<DenseStateId StateId, typename StateBase>
	inline StateBase& DenseStateStorage<StateId, StateBase>::Get(StateId id) {

		auto index = SakuEngine::DenseStateToIndex(id);
		if (!states_[index]) {
			ASSERT(FALSE, "State not found");
		}
		return *states_[index];
	}
} // SakuEngine
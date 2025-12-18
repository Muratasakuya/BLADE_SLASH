#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Object/State/DenseStateStorage.h>
#include <Engine/Object/State/StateNode.h>
#include <Engine/Object/State/StateIdConcepts.h>

// c++
#include <optional>
#include <concepts>
#include <utility>

namespace SakuEngine {

	//============================================================================
	//	BaseStateMachine class
	//	状態管理機械の基底クラス、遷移処理、更新を行う
	//============================================================================
	template <typename StateId, typename StateBase, typename Storage>
	class BaseStateMachine {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		BaseStateMachine() = default;
		~BaseStateMachine() = default;

		// 状態の追加
		template<class StateT, class... Args>
			requires std::derived_from<StateT, StateBase>&& std::constructible_from<StateT, Args...>
		StateT& Add(StateId id, Args&&... args) {

			return storage_.template Emplace<StateT>(id, std::forward<Args>(args)...);
		}

		// 状態入り初期化
		void SetEnter(StateId id);

		// 一連の状態処理を行う
		void Tick();
		// 非アクティブ状態の全状態に対して更新処理を行う
		void NonActiveTickAll();

		// 全状態に対して関数を実行
		template<class Fn>
		void ForEachState(Fn&& fn);

		//--------- accessor -----------------------------------------------------

		// 状態リクエスト
		void Request(StateId id) { requested_ = id; }

		// 現在の状態Idを取得
		StateId GetCurrentId() const noexcept { return currentId_; }
		// 現在の状態を取得
		StateBase& GetCurrent();
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 全ての状態の所有、管理
		Storage storage_;
		// 現在の状態
		StateBase* current_ = nullptr;

		// 現在の状態Id
		StateId currentId_{};
		// 遷移要求Id
		std::optional<StateId> requested_ = std::nullopt;

		//--------- functions ----------------------------------------------------

		// 状態内部の遷移要求を取得する
		std::optional<StateId> ConsumeFromState();

		// 状態を切り替える
		void ChangeTo(StateId next);
	};

	//============================================================================
	//	BaseStateMachine templateMethods
	//============================================================================

	template<typename StateId, typename StateBase, typename Storage>
	template<class Fn>
	inline void BaseStateMachine<StateId, StateBase, Storage>::ForEachState(Fn&& fn) {

		storage_.ForEach(std::forward<Fn>(fn));
	}

	template<typename StateId, typename StateBase, typename Storage>
	inline void BaseStateMachine<StateId, StateBase, Storage>::SetEnter(StateId id) {

		// 存在しない状態を処理しようとした場合はエラー
		ASSERT(storage_.Has(id), "StateMachine::Enter: State not found.");

		currentId_ = id;
		current_ = &storage_.Get(id);
		current_->Enter();
	}

	template<typename StateId, typename StateBase, typename Storage>
	inline void BaseStateMachine<StateId, StateBase, Storage>::Tick() {

		ASSERT(current_ != nullptr, "StateMachine::Enter: currentState is nullptr.");

		// 次の状態がリクエストされていれば切り替える
		if (requested_) {

			ChangeTo(*requested_);
			requested_.reset();
		}

		// 現在の状態を更新
		current_->Update();

		// 状態側からの遷移要求があれば切り替える
		if (auto next = ConsumeFromState(); next) {

			ChangeTo(*next);
		}
	}

	template<typename StateId, typename StateBase, typename Storage>
	inline void BaseStateMachine<StateId, StateBase, Storage>::NonActiveTickAll() {

		storage_.ForEach([&](StateBase& state) {
			if (&state != current_) {

				state.NonActiveUpdate();
			}
			});
	}

	template<typename StateId, typename StateBase, typename Storage>
	inline StateBase& BaseStateMachine<StateId, StateBase, Storage>::GetCurrent() {

		ASSERT(current_ != nullptr, "StateMachine::Enter: currentState is nullptr.");
		return *current_;
	}

	template<typename StateId, typename StateBase, typename Storage>
	inline std::optional<StateId> BaseStateMachine<StateId, StateBase, Storage>::ConsumeFromState() {

		if constexpr (requires(StateBase & state) { state.ConsumeRequested(); }) {

			return current_->ConsumeRequested();
		} else {

			return std::nullopt;
		}
	}

	template<typename StateId, typename StateBase, typename Storage>
	inline void BaseStateMachine<StateId, StateBase, Storage>::ChangeTo(StateId next) {

		// 同じ状態への遷移要求は無視
		if (next == currentId_) {
			return;
		}

		// 現在の処理を終了
		current_->Exit();

		// 次の状態へ遷移
		currentId_ = next;
		current_ = &storage_.Get(next);
		current_->Enter();
	}
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Object/State/DenseStateStorage.h>
#include <Engine/Object/State/StateNode.h>
#include <Engine/Object/State/StateIdConcepts.h>
#include <Engine/Object/State/StateMachineConfig.h>

// c++
#include <optional>
#include <concepts>
#include <utility>

namespace SakuEngine {

	// 状態を格納、管理を行う型の条件
	template <typename Storage, typename StateId, typename StateBase>
	concept StateStorage = requires(Storage state, StateId id) {
		{ state.Has(id) } -> std::same_as<bool>;
		{ state.Get(id) } -> std::same_as<StateBase&>;
		state.ForEach([](StateBase&) {});
	};

	//============================================================================
	//	BaseStateMachine class
	//	状態管理機械の基底クラス、遷移処理、更新を行う
	//============================================================================
	template <typename Config> requires HasStateMachineConfig<Config>
	class BaseStateMachine {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		BaseStateMachine() = default;
		~BaseStateMachine() = default;

		// 型エイリアス
		using StateId = typename Config::StateId;
		using StateBase = typename Config::StateBase;
		using Storage = typename Config::Storage;
		static_assert(StateStorage<Storage, StateId, StateBase>, "Storage interface mismatch.");

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
		// 強制状態遷移
		void Force(StateId next, bool restartIfSame = false);

		// 現在の状態Idを取得
		StateId GetCurrentId() const noexcept { return currentId_; }
		// 現在の状態を取得
		StateBase& GetCurrent();

		bool Has(StateId id) const { return storage_.Has(id); }
		StateBase& Get(StateId id) { return storage_.Get(id); }
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

	template<typename Config> requires HasStateMachineConfig<Config>
	inline void BaseStateMachine<Config>::SetEnter(StateId id) {

		ASSERT(storage_.Has(id), "StateMachine::SetEnter: State not found.");
		currentId_ = id;
		current_ = &storage_.Get(id);
		current_->Enter();
	}

	template<typename Config> requires HasStateMachineConfig<Config>
	inline void BaseStateMachine<Config>::Tick() {

		ASSERT(current_ != nullptr, "StateMachine::Tick: currentState is nullptr.");

		// 外部リクエストがあれば先に反映
		if (requested_) {

			ChangeTo(*requested_);
		}

		// 現在状態の更新
		current_->Update();

		// 状態処理ノード側からの遷移依頼
		if (auto next = ConsumeFromState(); next) {

			ChangeTo(*next);
		}
	}
	template<typename Config> requires HasStateMachineConfig<Config>
	inline void BaseStateMachine<Config>::NonActiveTickAll() {

		// 全非アクティブ状態の更新
		storage_.ForEach([&](StateBase& state) {
			if (&state != current_) {
				state.NonActiveUpdate();
			}
			});
	}

	template<typename Config> requires HasStateMachineConfig<Config>
	inline void BaseStateMachine<Config>::Force(StateId next, bool restartIfSame) {

		ASSERT(storage_.Has(next), "StateMachine::Force: State not found.");

		// 未初期化ならそのまま初期化して返す
		if (!current_) {
			SetEnter(next);
			return;
		}

		// 同じ状態の時
		if (next == currentId_) {

			// 同じ状態を処理できないなら何もしない
			if (!restartIfSame) {
				return;
			}

			// 現在状態の終了処理
			current_->Exit();
			// PreState設定可能なら設定する
			if constexpr (requires(StateBase & state, StateId prev) { state.SetPreState(prev); }) {
				current_->SetPreState(currentId_);
			}

			// 同じ状態へ遷移
			current_->Enter();
			requested_ = std::nullopt;
			return;
		}

		// 通常の強制遷移
		StateId preId = currentId_;
		current_->Exit();

		// 次の状態を取得
		currentId_ = next;
		current_ = &storage_.Get(next);

		// PreState設定可能なら設定する
		if constexpr (requires(StateBase & s, StateId prevId) { s.SetPreState(prevId); }) {
			current_->SetPreState(preId);
		}

		// 次状態へ遷移
		current_->Enter();
		requested_ = std::nullopt;
	}

	template<typename Config> requires HasStateMachineConfig<Config>
	inline BaseStateMachine<Config>::StateBase& BaseStateMachine<Config>::GetCurrent() {

		ASSERT(current_ != nullptr, "StateMachine::GetCurrent: currentState is nullptr.");
		return *current_;
	}

	template<typename Config> requires HasStateMachineConfig<Config>
	inline std::optional<typename BaseStateMachine<Config>::StateId> BaseStateMachine<Config>::ConsumeFromState() {

		using StateBase = typename BaseStateMachine<Config>::StateBase;
		if constexpr (requires(StateBase & state) { state.ConsumeRequested(); }) {

			return current_->ConsumeRequested();
		} else {

			return std::nullopt;
		}
	}

	template<typename Config> requires HasStateMachineConfig<Config>
	inline void BaseStateMachine<Config>::ChangeTo(StateId next) {

		// 同じ状態なら何もしない
		if (next == currentId_) {
			return;
		}

		ASSERT(storage_.Has(next), "StateMachine::ChangeTo: State not found.");

		// 現在状態の終了処理
		StateId preId = currentId_;
		current_->Exit();

		// 次の状態を取得
		currentId_ = next;
		current_ = &storage_.Get(next);

		// PreState設定可能なら設定する
		if constexpr (requires(StateBase & s, StateId prevId) { s.SetPreState(prevId); }) {
			current_->SetPreState(preId);
		}

		// 次状態へ遷移
		current_->Enter();
	}

	template<typename Config> requires HasStateMachineConfig<Config>
	template<class Fn>
	inline void BaseStateMachine<Config>::ForEachState(Fn&& fn) {

		storage_.ForEach(std::forward<Fn>(fn));
	}
} // SakuEngine
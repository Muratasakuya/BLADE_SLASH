#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/DenseStateStorage.h>
#include <Engine/Object/State/StateIdConcepts.h>
#include <Engine/Object/State/StateNode.h>

namespace SakuEngine {

	// 状態機械設定構造体
	template <typename TStateId, typename TStateBase, typename TStorage>
	struct StateMachineConfig {

		using StateId = TStateId;
		using StateBase = TStateBase;
		using Storage = TStorage;
	};

	// StateMachineConfigの要件を満たす型
	template <typename Config>
	concept HasStateMachineConfig = requires {

		typename Config::StateId;
		typename Config::StateBase;
		typename Config::Storage;
	};

	// StateNodeを継承した状態を持つ、DenseStateStorageを使用する状態機械設定
	template<DenseStateId Id, typename Base = StateNode<Id>>
	using DenseStateConfig = StateMachineConfig<Id, Base, DenseStateStorage<Id, Base>>;
}; // SakuEngine
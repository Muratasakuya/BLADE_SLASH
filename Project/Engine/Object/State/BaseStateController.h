#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/BaseStateMachine.h>

namespace SakuEngine {

	//============================================================================
	//	BaseStateController class
	//	状態機械の制御を行う基底クラス
	//============================================================================
	template<typename Derived, typename StateId, typename StateBase, typename Storage>
	class BaseStateController {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		BaseStateController() = default;
		~BaseStateController() = default;

		// 状態機械の型エイリアス
		using Machine = BaseStateMachine<StateId, StateBase, Storage>;

		// 一連の状態処理を行う
		void Tick();

		// 非アクティブ状態の全状態に対して更新処理を行う
		void NonActiveTickAll() { machine_.NonActiveTickAll(); }

		//--------- accessor -----------------------------------------------------

	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		Machine& GetMachine() { return machine_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		Machine machine_;
	};

	//============================================================================
	//	BaseStateController templateMethods
	//============================================================================

	template<typename Derived, typename StateId, typename StateBase, typename Storage>
	inline void BaseStateController<Derived, StateId, StateBase, Storage>::Tick() {

		// 外部からの遷移要求を決定
		static_cast<Derived*>(this)->DecideExternalTransition();
		// 状態機械のTickを実行
		machine_.Tick();
	}
}
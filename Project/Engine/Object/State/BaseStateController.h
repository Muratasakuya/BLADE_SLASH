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
	template <typename Config> requires HasStateMachineConfig<Config>
	class BaseStateController {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		BaseStateController() = default;
		virtual ~BaseStateController() = default;

		// 型エイリアス
		using StateId = typename Config::StateId;
		using StateBase = typename Config::StateBase;
		using Machine = BaseStateMachine<Config>;

		// 一連の状態処理を行う
		void Tick();

		// 非アクティブ状態の全状態に対して更新処理を行う
		void NonActiveTickAll() { machine_.NonActiveTickAll(); }
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- accessor -----------------------------------------------------

		Machine& GetMachine() { return machine_; }
		const Machine& GetMachine() const { return machine_; }

		//--------- functions -----------------------------------------------------

		// 外部遷移判定を行う
		virtual void DecideExternalTransition() = 0;
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

	template<typename Config> requires HasStateMachineConfig<Config>
	inline void BaseStateController<Config>::Tick() {

		// 外部遷移判定
		DecideExternalTransition();
		// 状態遷移、更新処理
		machine_.Tick();
	}
}
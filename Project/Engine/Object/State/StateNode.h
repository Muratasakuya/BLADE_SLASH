#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <type_traits>
#include <optional>

namespace SakuEngine {

	//============================================================================
	//	StateNode class
	//	遷移要求を行う
	//============================================================================
	template <typename T>
	class StateNode {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		StateNode() = default;
		virtual ~StateNode() = default;

		// 更新処理
		virtual void Update() = 0;

		// 状態遷移開始
		virtual void Enter() {}
		// 状態遷移終了
		virtual void Exit() {}

		//--------- accessor -----------------------------------------------------

		// 前状態の設定
		void SetPreviousState(T previous) noexcept { previousState_ = previous; }

		// 状態処理側からの遷移要求を返し、リセットする
		std::optional<T> ConsumeRequested() noexcept;
	protected:

		// 前状態の取得
		T GetPreviousState() const noexcept { return previousState_; }
		// 遷移要求を取得
		void Request(T request) noexcept { requested_ = request; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 前状態
		T previousState_;
		// 遷移要求を行う状態
		std::optional<T> requested_;
	};

	//============================================================================
	//	StateNode templateMethods
	//============================================================================

	template<typename T>
	inline std::optional<T> StateNode<T>::ConsumeRequested() noexcept {

		// 遷移要求を取得し、リセットする
		std::optional<T> request = requested_;
		requested_ = std::nullopt;
		return  request;
	}
}; // SakuEngine
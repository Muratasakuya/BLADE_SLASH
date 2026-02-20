#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyBaseAttackState class
//	ボスの攻撃状態の基底クラス
//============================================================================
class BossEnemyBaseAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyBaseAttackState() = default;
	~BossEnemyBaseAttackState() = default;

	virtual void CreateEffect() {}

	virtual void Enter() {}

	virtual void Update() {}
	virtual void UpdateAlways() {}

	virtual void Exit() {}

	// imgui
	virtual void ImGui() {}

	// json
	virtual void ApplyJson([[maybe_unused]] const Json& data) {}
	virtual void SaveJson([[maybe_unused]] Json& data) {}
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 攻撃予兆倍速
	float attackSpeedRate_ = 1.0f;

	//--------- functions ----------------------------------------------------

	// プレイヤーの状態をチェックして、パリィ待機状態になっていれば倍速を適用する
	void CheckAndApplySpeedRate();

	// リセット
	void ResetSpeedRate();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 倍速適用済みかどうか
	bool isAppliedParrySpeedRate_ = false;
};
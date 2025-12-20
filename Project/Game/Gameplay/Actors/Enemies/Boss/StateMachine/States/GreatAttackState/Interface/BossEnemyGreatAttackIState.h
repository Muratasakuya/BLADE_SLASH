#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/Effect/User/EffectGroup.h>
#include <Game/Objects/GameScene/Enemy/Boss/Structures/BossEnemyStructures.h>

// imgui
#include <imgui.h>
// front
class BossEnemy;
class Player;
class FollowCamera;
class BossEnemyGreatAttackState;
class BossEnemyAttackSign;

//============================================================================
//	BossEnemyGreatAttackIState class
//	大技攻撃状態の各状態のインターフェース
//============================================================================
class BossEnemyGreatAttackIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackIState() = default;
	virtual ~BossEnemyGreatAttackIState() = default;

	// 共通初期化
	void Init(BossEnemy* bossEnemy, Player* player, FollowCamera* followCamera);

	// 状態遷移時
	virtual void Enter() = 0;

	// 更新処理
	virtual void Update() = 0;
	virtual void UpdateAlways() {}

	// 状態終了時
	virtual void Exit() = 0;

	// imgui
	virtual void ImGui() = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetParentState(BossEnemyGreatAttackState* parentState) { parentState_ = parentState; }
	void SetAttackSign(BossEnemyAttackSign* attackSign) { attackSign_ = attackSign; }

	// 状態遷移可能か
	bool CanExit() const { return canExit_; }
	const ParryParameter& GetParryParam() const { return parryParam_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Player* player_;
	FollowCamera* followCamera_;
	BossEnemy* bossEnemy_;
	BossEnemyGreatAttackState* parentState_;
	BossEnemyAttackSign* attackSign_;

	// 状態終了フラグ
	bool canExit_ = false;

	// パリィパラメーター
	ParryParameter parryParam_;

	//--------- functions ----------------------------------------------------

};
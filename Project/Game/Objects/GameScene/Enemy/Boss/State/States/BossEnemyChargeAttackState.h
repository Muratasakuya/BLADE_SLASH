#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>
#include <Game/Objects/GameScene/Enemy/Boss/Collision/BossEnemyBladeCollision.h>

//============================================================================
//	BossEnemyChargeAttackState class
//	チャージ攻撃状態
//============================================================================
class BossEnemyChargeAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyChargeAttackState();
	~BossEnemyChargeAttackState() = default;

	void Enter(BossEnemy& bossEnemy) override;

	void Update(BossEnemy& bossEnemy) override;
	void UpdateAlways(BossEnemy& bossEnemy) override;

	void Exit(BossEnemy& bossEnemy) override;

	// imgui
	void ImGui(const BossEnemy& bossEnemy) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// parameters
	float exitTimer_; // 遷移可能にするまでの経過時間
	float exitTime_;  // 遷移可能にするまでの時間

	// 1本の刃
	std::unique_ptr<BossEnemyBladeCollision> singleBlade_;
	float singleBladeMoveSpeed_; // 刃の進む速度
	// エフェクト
	float singleBladeEffectScalingValue_;

	//--------- functions ----------------------------------------------------

	// update
	void UpdateBlade(BossEnemy& bossEnemy);

	// helper
	SakuEngine::Vector3 CalcBaseDir(const BossEnemy& bossEnemy) const;
};
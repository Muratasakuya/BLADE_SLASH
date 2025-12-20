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

	BossEnemyChargeAttackState() = default;
	~BossEnemyChargeAttackState() = default;

	void CreateEffect() override;

	void Enter() override;

	void Update() override;
	void UpdateAlways() override;

	void Exit() override;

	// imgui
	void ImGui() override;

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
	void UpdateBlade();

	// helper
	SakuEngine::Vector3 CalcBaseDir() const;
};
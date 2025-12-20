#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/EffectGroup.h>
#include <Engine/Utility/Enum/Easing.h>
#include <Engine/MathLib/MathUtils.h>
#include <Engine/Object/State/StateNode.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Structures/BossEnemyStructures.h>

// front
class BossEnemy;
class BossEnemyAttackSign;
class Player;
class FollowCamera;

//============================================================================
//	BossEnemyIState class
//	ボスの状態インターフェース
//============================================================================
class BossEnemyIState :
	public SakuEngine::StateNode<BossEnemyState> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyIState() = default;
	virtual ~BossEnemyIState() = default;

	// エフェクトの生成
	virtual void CreateEffect() {}

	// 常に行う更新処理
	virtual void Update() override = 0;
	virtual void UpdateAlways() {}

	// エディター
	virtual void ImGui() = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(BossEnemy* bossEnemy) { bossEnemy_ = bossEnemy; }
	void SetPlayer(Player* player) { player_ = player; }
	void SetFollowCamera(FollowCamera* followCamera) { followCamera_ = followCamera; }
	virtual void SetAttackSign(BossEnemyAttackSign* attackSign) { attackSign_ = attackSign; }

	virtual bool GetCanExit() const { return canExit_; }
	const ParryParameter& GetParryParam() const { return parryParam_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 状態遷移対象
	BossEnemy* bossEnemy_;

	Player* player_;
	FollowCamera* followCamera_;
	BossEnemyAttackSign* attackSign_;

	// 共通parameters
	float nextAnimDuration_; // 次のアニメーション遷移にかかる時間
	bool canExit_ = true;    // 遷移可能かどうか
	float rotationLerpRate_; // 回転補完割合
	ParryParameter parryParam_;
};
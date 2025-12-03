#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/Effect/User/EffectGroup.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/Interface/BossEnemyGreatAttackIState.h>

//============================================================================
//	BossEnemyGreatAttackCharge class
//	チャージ攻撃の準備中...状態
//============================================================================
class BossEnemyGreatAttackCharge :
	public BossEnemyGreatAttackIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackCharge();
	~BossEnemyGreatAttackCharge() = default;

	// 状態遷移時
	void Enter() override;

	// 更新処理
	void Update() override;
	void UpdateAlways() override;

	// 状態終了時
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

	// 次の状態進むまでの時間
	StateTimer nextTimer_;

	// チャージ
	std::unique_ptr<EffectGroup> beginChargeEffect_;
	Vector3 beginChargeEffectOffset_;
	bool emitedBeginChargeEffect_ = false;

	// アニメーション遷移時間
	float nextAnimDuration_;

	// skyboxの色
	Color skyboxColor_;

	//--------- functions ----------------------------------------------------

};
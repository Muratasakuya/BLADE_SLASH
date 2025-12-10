#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/ConicalPendulum.h>
#include <Engine//Utility/Animation/SimpleAnimation.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/Interface/BossEnemyGreatAttackIState.h>

//============================================================================
//	BossEnemyGreatAttackApproach class
//============================================================================
class BossEnemyGreatAttackApproach :
	public BossEnemyGreatAttackIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackApproach();
	~BossEnemyGreatAttackApproach() = default;

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

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		Begin,    // 溜め
		Approach, // 近接
		Attack    // 攻撃
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 最初の補間移動
	SakuEngine::SimpleAnimation<SakuEngine::Vector3> startMoveAnim_;

	// 振り子移動
	SakuEngine::ConicalPendulum movePendulum_;
	// 振り子の親からのオフセット位置
	SakuEngine::Vector3 pendulumOffset_;
	// 円錐のX軸回転オフセット
	float pendulumRotateX_;

	// 振り子移動での角への最大到達回数
	uint32_t pendulumMaxReachCount_;
	uint32_t prevPendulumReachCount_;

	// 最初の溜め時間
	SakuEngine::StateTimer beginTimer_;

	// 剣エフェクト
	// 左から右
	std::unique_ptr<SakuEngine::EffectGroup> leftToRightSlashEffect_;
	// 右から左
	std::unique_ptr<SakuEngine::EffectGroup> rightToLeftSlashEffect_;
	// オフセット位置
	SakuEngine::Vector3 slashEffectOffset_;

	//--------- functions ----------------------------------------------------

	// 状態毎の更新
	void UpdateBegin();
	void UpdateApproach();
	void UpdateAttack();

	// カウントに合わせたアニメーション再生
	void StartPendulumAnim();
};
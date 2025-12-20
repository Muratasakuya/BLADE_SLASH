#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Helper/SlashEffectHelper.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyContinuousAttackState class
//	連続剣撃状態、3回連続で突進攻撃を行う
//============================================================================
class BossEnemyContinuousAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyContinuousAttackState() = default;
	~BossEnemyContinuousAttackState() = default;

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

	//--------- stricture ----------------------------------------------------

	// 状態
	enum class State {

		ParrySign, // パリィ受付中
		Attack     // 攻撃
	};

	//--------- variables ----------------------------------------------------

	// 状態
	State currentState_;

	SakuEngine::Vector3 startPos_; // 開始座標
	uint32_t keyEventIndex_;

	// parameters
	float lerpTimer_;       // 座標補間の際の経過時間
	float lerpTime_;        // 座標補間の際の時間
	EasingType easingType_; // 補間の際のイージング

	float attackOffsetTranslation_; // 座標からのオフセット距離

	float exitTimer_; // 遷移可能にするまでの経過時間
	float exitTime_;  // 遷移可能にするまでの時間
	bool reachedPlayer_; // 近くまで来たかどうか

	// 剣エフェクト
	SakuEngine::SlashEffectHelper firstSlash_;  // 1回目
	SakuEngine::SlashEffectHelper secondSlash_; // 2回目
	SakuEngine::SlashEffectHelper thirdSlash_;  // 3回目

	// debug
	bool parried_;
	uint32_t emitCount_;

	//--------- functions ----------------------------------------------------

	// update
	void UpdateParrySign();
	void UpdateAttack();
	void UpdateParryTiming();
	void UpdateEffectEvent();
};
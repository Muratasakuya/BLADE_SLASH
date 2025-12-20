#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>

//============================================================================
//	BossEnemyJumpAttackState class
//	ジャンプ攻撃状態
//============================================================================
class BossEnemyJumpAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyJumpAttackState() = default;
	~BossEnemyJumpAttackState() = default;

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

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		Pre,
		Jump
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 座標の補間、XとZだけ
	SakuEngine::SimpleAnimation<SakuEngine::Vector3> lerpTranslationXZ_;
	float targetDistance_; // プレイヤーから離す距離
	// Y座標のジャンプ補間
	const uint32_t jumpKeyframeCount_ = 4;
	std::vector<SakuEngine::Vector3> jumpKeyframes_;
	EasingType jumpEasing_;

	// 剣エフェクト
	SlashEffect slash_;

	//--------- functions ----------------------------------------------------

	// update
	// 予備動作
	void UpdatePre();
	// ジャンプ
	void UpdateJump();

	// helper
	// 補間座標の設定
	void SetLerpTranslation();
};
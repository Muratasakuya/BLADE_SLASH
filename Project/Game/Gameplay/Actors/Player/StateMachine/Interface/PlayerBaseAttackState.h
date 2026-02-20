#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>
#include <Game/Gameplay/Actors/Player/StateMachine/Interface/PlayerIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	PlayerBaseAttackState class
//	IStateを継承した攻撃系の基底クラス
//============================================================================
class PlayerBaseAttackState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerBaseAttackState() = default;
	~PlayerBaseAttackState() = default;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	float exitTimer_; // 経過時間
	float exitTime_;  // 次に状態に遷移できるまでの時間

	float attackPosLerpCircleRange_; // 攻撃したとき指定の座標まで補間する範囲(円の半径)
	float attackLookAtCircleRange_;  // 攻撃したとき敵の方向を向く範囲(円の半径)
	float attackOffsetTranslation_;  // 敵の座標からのオフセット距離
	float attackPosLerpTimer_;       // 座標補間の際の経過時間
	float attackPosLerpTime_;        // 座標補間にかける時間
	EasingType attackPosEaseType_;   // 座標補間に使用するイージングの種類

	// 補間目標
	std::optional<SakuEngine::Vector3> targetTranslation_;
	std::optional<SakuEngine::Quaternion> targetRotation_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson(const Json& data);
	void SaveJson(Json& data);

	// imgui
	void ImGui();

	// 攻撃アシスト、座標補間、方向補間を敵との距離に応じて処理する
	void AttackAssist(bool onceTarget = false, bool isOnlyAssistRotation = false);

	// 範囲内に入っているかチェックする
	bool CheckInRange(float range, float distance);

	// debug
	void DrawAttackOffset();
};
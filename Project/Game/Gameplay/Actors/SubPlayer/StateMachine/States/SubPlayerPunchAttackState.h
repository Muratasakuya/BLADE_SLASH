#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/KeyframeObject3D.h>
#include <Engine/Editor/Effect/User/EffectGroup.h>
#include <Engine/Utility/Animation/AnimationLoop.h>
#include <Game/Gameplay/Actors/SubPlayer/StateMachine/Interface/SubPlayerIState.h>

//============================================================================
//	SubPlayerPunchAttackState class
//============================================================================
class SubPlayerPunchAttackState :
	public SubPlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SubPlayerPunchAttackState();
	~SubPlayerPunchAttackState() = default;

	// 状態遷移時
	void Enter() override;

	// 更新処理
	void Update() override;
	void UpdateAlways() override;

	// 状態終了時
	void Exit() override;

	// エディター
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	// 最後の殴りまで行ったか
	bool IsFinishPunchAttack() const { return currentState_ == State::Leave; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		Approach, // 近づく
		Attack,   // 攻撃
		Leave,    // 離れる
	};

	// 攻撃情報
	struct AttackInfo {

		SakuEngine::Vector3 startPos;  // 開始位置
		SakuEngine::Vector3 targetPos; // 目標位置

		SakuEngine::Vector3 chargeStartPos; // 溜め開始位置
		SakuEngine::Vector3 chargeTargetPos; // 溜め目標位置

		bool isActive = false;
		SakuEngine::StateTimer timer;   // 攻撃タイマー
		SakuEngine::AnimationLoop loop; // ループ
		float prevRawT = 0.0f;
	};

	//--------- variables ----------------------------------------------------

	// 追加したキーの情報
	const std::string& addKeyColor = "Color";

	// 現在の状態
	State currentState_;

	// 各パーツのキーフレーム移動
	// 体
	std::unique_ptr<SakuEngine::KeyframeObject3D> bodyApproachKeyframeObject_;
	std::unique_ptr<SakuEngine::KeyframeObject3D> bodyLeaveKeyframeObject_;
	// 右手
	std::unique_ptr<SakuEngine::KeyframeObject3D> rightHandApproachKeyframeObject_;
	std::unique_ptr<SakuEngine::KeyframeObject3D> rightHandLeaveKeyframeObject_;
	// 左手
	std::unique_ptr<SakuEngine::KeyframeObject3D> leftHandApproachKeyframeObject_;
	std::unique_ptr<SakuEngine::KeyframeObject3D> leftHandLeaveKeyframeObject_;

	// 攻撃に使用する変数
	uint32_t attackCount_;            // 何回攻撃するか
	float attackDuration_;            // 攻撃時間
	float attackMoveDistance_;        // 攻撃手の移動距離
	float bossEnemyOffsetY_;          // ボス敵の高さオフセット

	SakuEngine::StateTimer leftHandAttackDelayTimer_; // 手攻撃の遅延、左手を遅延させる
	SakuEngine::StateTimer chargeTimer_;              // 溜め時間
	SakuEngine::StateTimer chargeAttackTimer_;        // 溜め後の攻撃

	// 体
	float bodyOffsetAngleY_;        // 体の向き補正角度Y
	SakuEngine::Quaternion enterBodyRotation_;  // 侵入時の回転
	SakuEngine::Quaternion bodyStartRotation_;  // 開始回転
	SakuEngine::Quaternion bodyTargetRotation_; // 目標回転
	// 手
	AttackInfo rightHandAttackInfo_;  // 右手攻撃情報
	AttackInfo leftHandAttackInfo_;   // 左手攻撃情報

	// ヒットエフェクト
	std::unique_ptr<SakuEngine::EffectGroup> hitEffect_;

	//--------- functions ----------------------------------------------------

	// 状態別更新処理
	void UpdateApproach();
	void UpdateAttack();
	void UpdateLeave();

	// 常にキーフレームオブジェクトを更新
	void UpdateKeyframeObjects();
	// キーオブジェクトのリセット
	void ResetKeyframeObjects();

	// 補間の更新とトランスフォームの更新
	void UpdateKeyAndApply(SakuEngine::KeyframeObject3D& bodyKeyframe,
		SakuEngine::KeyframeObject3D& rightHandKeyframe, SakuEngine::KeyframeObject3D& leftHandKeyframe);
	// 全ての補間処理が終了したかのチェック
	bool IsAllKeyframeEnd(SakuEngine::KeyframeObject3D& bodyKeyframe,
		SakuEngine::KeyframeObject3D& rightHandKeyframe, SakuEngine::KeyframeObject3D& leftHandKeyframe);

	// 手の補間
	void LerpAttackHand(AttackInfo& attackInfo, SakuEngine::GameObject3D& hand);
	void LerpChargeHand(AttackInfo& attackInfo, SakuEngine::GameObject3D& hand);
	// 攻撃情報のセットアップ
	void SetupAttackInfo(AttackInfo& attackInfo, const SakuEngine::GameObject3D& hand, bool isAttackHand);
};
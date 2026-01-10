#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/KeyframeObject3D.h>
#include <Engine/Editor/Effect/User/EffectGroup.h>
#include <Engine/Utility/Timer/DelayedHitstop.h>
#include <Game/Gameplay/Actors/Player/Effect/PlayerAfterImageEffect.h>
#include <Game/Gameplay/Actors/Player/StateMachine/Interface/PlayerBaseAttackState.h>

//============================================================================
//	PlayerSkilAttackState class
//	スキル攻撃
//============================================================================
class PlayerSkilAttackState :
	public PlayerBaseAttackState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerSkilAttackState() = default;
	~PlayerSkilAttackState() = default;

	void CreateEffect() override;

	void Enter() override;

	void Update() override;
	void BeginUpdateAlways() override;
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

		MoveAttack, // 移動攻撃
		JumpAttack, // ジャンプ攻撃
	};

	// ヒットストップ
	struct HitStop {

		bool isStarted = false;
		float startProgress;    // 発生させる攻撃進捗
		SakuEngine::DelayedHitstop hitStop; // ヒットストップ
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 座標移動のキーフレーム
	std::unique_ptr<SakuEngine::KeyframeObject3D> moveKeyframeObject_;
	// ジャンプ移動のキーフレーム
	std::unique_ptr<SakuEngine::KeyframeObject3D> jumpKeyframeObject_;
	// 空の親トランスフォーム、敵が範囲内にいないときに参照する親
	SakuEngine::Transform3D* moveFrontTransform_;
	// タグ
	SakuEngine::ObjectTag* moveFrontTag_;
	// 敵のトランスフォームの値を補正する用のトランスフォーム
	SakuEngine::Transform3D* fixedEnemyTransform_;
	// タグ
	SakuEngine::ObjectTag* fixedEnemyTag_;

	// 回転の軸
	SakuEngine::Vector3 rotationAxis_;
	// 移動の前座標
	SakuEngine::Vector3 preMovePos_;

	// 目標への回転
	SakuEngine::Quaternion targetRotation_;

	// ジャンプ攻撃アニメーションへの遷移時間
	float nextJumpAnimDuration_;

	// 範囲内にいるかどうか
	bool isInRange_;

	// 攻撃ヒットストップ
	HitStop moveAttackHitstop_;
	HitStop jumpAttackHitstop_;

	// 残像表現エフェクト
	std::unique_ptr<PlayerAfterImageEffect> afterImageEffect_;

	// 移動エフェクト
	std::unique_ptr<SakuEngine::EffectGroup> moveAtackEffect_;
	float jumpEffectEmitProgress_; // ジャンプエフェクトを発生させるタイミング
	bool jumpMoveEffectEmitted_ = false;
	// 地割れエフェクト
	std::unique_ptr<SakuEngine::EffectGroup> groundCrackEffect_;
	bool groundCrackEmitted_ = false;

	// 移動攻撃SE音量
	float moveAttackSEVolume_ = 1.0f;

	//--------- functions ----------------------------------------------------

	// 状態毎の更新
	void UpdateMoveAttack();
	void UpdateJumpAttack();

	// 範囲内チェックを行って補間目標を設定する
	void SetTargetByRange(SakuEngine::KeyframeObject3D& keyObject, const std::string& cameraKeyName);
};
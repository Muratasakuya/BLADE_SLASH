#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Effect/User/EffectGroup.h>
#include <Engine/Utility/Timer/DelayedHitstop.h>
#include <Engine/Input/InputStructures.h>
#include <Game/Gameplay/Actors/Player/StateMachine/Interface/PlayerBaseAttackState.h>

//============================================================================
//	PlayerAttack_4thState class
//	4段目の攻撃
//============================================================================
class PlayerAttack_4thState :
	public PlayerBaseAttackState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAttack_4thState() = default;
	~PlayerAttack_4thState() = default;

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

	//--------- accessor -----------------------------------------------------

	bool GetCanExit() const override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ヒットストップ
	struct HitStop {

		bool isStart = false;               // 開始フラグ
		float startProgress;                // 発生させる攻撃進捗
		SakuEngine::DelayedHitstop hitStop; // ヒットストップ
	};

	//--------- variables ----------------------------------------------------

	bool assisted_;

	// 入力振動
	InputVibrationParams vibrationParams_;

	// parameters
	// 座標補間を行わないときの処理
	SakuEngine::StateTimer moveTimer_;
	float moveValue_;   // 移動量
	SakuEngine::Vector3 startPos_;  // 開始座標
	SakuEngine::Vector3 targetPos_; // 目標座標

	// 地割れエフェクト
	std::unique_ptr<SakuEngine::EffectGroup> groundCrackEffect_;
	bool groundCrackEmitted_ = false;
	// 回転エフェクト
	std::unique_ptr<SakuEngine::EffectGroup> rotationEffect_;
	SakuEngine::Vector3 rotateEffectOffset_;
	SakuEngine::Vector3 rotateEffectOffsetRotation_;

	// 攻撃ヒットストップ
	HitStop attackHitStop_;
};
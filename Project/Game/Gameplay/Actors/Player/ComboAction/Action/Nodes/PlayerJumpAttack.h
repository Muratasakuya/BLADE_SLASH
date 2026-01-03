#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Helper/BasePlayerMoveKeyAttack.h>

//============================================================================
//	PlayerJumpAttack class
//	プレイヤーのジャンプ攻撃ノード
//============================================================================
class PlayerJumpAttack :
	public BasePlayerMoveKeyAttack {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerJumpAttack();
	~PlayerJumpAttack() = default;

	void Enter() override;

	void Update() override;
	void UpdateAlways() override;

	void Exit() override;

	void ImGui() override;

	void FromJson(const Json& data) override;
	void ToJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetProgress(float progress) override;
	bool IsFinished() const override;
	float GetTotalTime() const override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// アニメーションの遷移時間
	float nextAnimDuration_;
	// 処理終了後の待機時間タイマー
	SakuEngine::StateTimer exitTimer_;

	// ジャンプ移動のキーフレーム
	std::unique_ptr<SakuEngine::KeyframeObject3D> jumpKeyframeObject_;

	//--------- functions ----------------------------------------------------

};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Interface/PlayerIActionNode.h>

//============================================================================
//	PlayerCatchWeaponAttack class
//	プレイヤーの武器回収攻撃ノード
//============================================================================
class PlayerCatchWeaponAttack :
	public PlayerIActionNode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerCatchWeaponAttack() = default;
	~PlayerCatchWeaponAttack() = default;

	void Enter() override;

	void Update() override;

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

	// 座標移動補間
	SakuEngine::LerpValue<SakuEngine::Vector3> lerpPos_;

	//--------- functions ----------------------------------------------------

};
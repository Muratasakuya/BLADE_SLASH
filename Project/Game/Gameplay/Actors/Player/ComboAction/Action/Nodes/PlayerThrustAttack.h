#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Helper/BasePlayerMoveKeyAttack.h>

//============================================================================
//	PlayerThrustAttack class
//	プレイヤーの突き攻撃ノード
//============================================================================
class PlayerThrustAttack :
	public BasePlayerMoveKeyAttack {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerThrustAttack();
	~PlayerThrustAttack() = default;

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

	// 座標移動のキーフレーム
	std::unique_ptr<SakuEngine::KeyframeObject3D> moveKeyframeObject_;

	// 処理の終了を返すフラグ
	bool isFinished_ = false;

	//--------- functions ----------------------------------------------------

};
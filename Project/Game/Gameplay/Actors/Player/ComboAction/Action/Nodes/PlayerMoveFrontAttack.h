#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Interface/PlayerIActionNode.h>

//============================================================================
//	PlayerMoveFrontAttack class
//	プレイヤーの前移動攻撃ノード
//============================================================================
class PlayerMoveFrontAttack :
	public PlayerIActionNode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerMoveFrontAttack() = default;
	~PlayerMoveFrontAttack() = default;

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

	//--------- structure ----------------------------------------------------

	// 処理終了条件
	enum class EndCondition {

		EndAnim,    // アニメーション再生完了
		LerpPos,    // 座標補間完了
		LerpRotate, // 回転補間完了
	};

	//--------- variables ----------------------------------------------------

	// 処理終了条件
	EndCondition endCondition_;

	// 処理開始時に再生するアニメーションの名前
	std::string animationName_ = "player_idle";
	// アニメーションの遷移時間
	float nextAnimDuration_;

	// 敵との距離
	float distance_;

	// 座標移動補間
	SakuEngine::LerpValue<SakuEngine::Vector3> lerpPos_;
	// 回転補間
	SakuEngine::LerpValue<SakuEngine::Quaternion> lerpRotate_;

	// 処理終了後の待機時間タイマー
	SakuEngine::StateTimer exitTimer_;

	// 外部からポーズを設定された場合の情報
	std::string oldAnimName_;
	float oldAnimTime_ = 0.0f;

	//--------- functions ----------------------------------------------------

	// 終了条件の時間が過ぎたか
	bool IsEndConditionReached() const;
};
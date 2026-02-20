#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Interface/PlayerIActionNode.h>

//============================================================================
//	PlayerMoveWaysAttack class
//	プレイヤーの多方向移動攻撃ノード
//============================================================================
class PlayerMoveWaysAttack :
	public PlayerIActionNode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerMoveWaysAttack() = default;
	~PlayerMoveWaysAttack() = default;

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

	static constexpr size_t kNumSegments = 3;

	// 敵との距離に応じた処理を行うフラグ
	bool approachPhase_;
	bool loopApproach_ = false;

	// 移動パラメータ
	SakuEngine::Vector3 startTranslation_;                    // 移動開始地点
	std::array<SakuEngine::Vector3, kNumSegments> wayPoints_; // 移動する点の数
	size_t currentIndex_; // 現在の区間
	float segmentTimer_;  // 区間経過時間
	float segmentTime_;   //区間にかかる時間
	float attackPosLerpTime_; // 攻撃位置補間時間
	float swayRate_;      //揺れ幅
	// 敵との距離
	float offsetDistance_;

	float leftPointAngle_;  // 左の座標の角度
	float rightPointAngle_; // 右の座標の角度

	// 座標補間を行わないときの処理
	float approachForwardDistance_; // 前方に進む距離
	float approachSwayLength_;      // 左右振れ幅
	float approachLeftPointAngle_;  // 左の角度
	float approachRightPointAngle_; // 右の角度

	// デバッグ表示
	std::array<SakuEngine::Vector3, kNumSegments> debugWayPoints_;
	std::array<SakuEngine::Vector3, kNumSegments> debugApproachWayPoints_;

	// 回転補間
	SakuEngine::LerpValue<SakuEngine::Quaternion> lerpRotate_;

	// 補間目標座標
	std::optional<SakuEngine::Vector3> targetTranslation_;
	// イージングタイプ
	EasingType attackPosEaseType_;

	//--------- functions ----------------------------------------------------

	// 補間点計算
	void CalcWayPoints(std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints);
	// 目標地点への補間点計算
	void CalcWayPointsToTarget(const SakuEngine::Vector3& start, const SakuEngine::Vector3& target,
		float leftT, float rightT, float swayLength,
		std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints);
	// リアクションエリア外への補間点計算
	void CalcApproachWayPoints(std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints);
	// 区間補間処理
	bool LerpAlongSegments();
};
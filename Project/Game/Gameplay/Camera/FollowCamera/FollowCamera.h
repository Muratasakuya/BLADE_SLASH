#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Game/Gameplay/Actors/Player/Structure/PlayerStructures.h>
#include <Game/Gameplay/Camera/FollowCamera/StateMachine/FollowCameraStateMachine.h>

//============================================================================
//	FollowCamera class
//	追従カメラ
//============================================================================
class FollowCamera :
	public SakuEngine::BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCamera() = default;
	~FollowCamera() = default;

	void Init() override;
	void LoadAnim();

	void Update() override;

	void ImGui() override;

	// 最初にカメラが一気に移動してしまうのを防ぐ
	void UpdateInitialSettings();

	//--------- accessor -----------------------------------------------------

	void SetAnchorObject(const SakuEngine::GameObject3D* anchor);
	void SetLookAtTargetObject(const SakuEngine::GameObject3D* lookAtTarget);

	void WarmStart() { stateMachine_->WarmStartFollow(); }
	bool IsFinishedHandoffBlend() const { return  stateMachine_->IsFinishedHandoffBlend(); }

	// エディターによるカメラアニメーション
	void StartPlayerActionAnim(PlayerState state);
	void StartPlayerActionAnim(const std::string& animName);
	void EndPlayerActionAnim(bool isWarmStart);

	// 視点を注視点に向ける
	void StartLookToTarget(bool isReset = false, bool isLockTarget = false,
		std::optional<float> targetXRotation = std::nullopt, float lookTimerRate = 1.0f);
	void SetLookAlwaysTarget(bool look) { lookAlwaysTarget_ = look; }

	// ブレンド処理が終了しているか

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 状態の管理
	std::unique_ptr<FollowCameraStateMachine> stateMachine_;
	const SakuEngine::GameObject3D* anchorObject_;       // 基準点
	const SakuEngine::GameObject3D* lookAtTargetObject_; // 注視点

	// 視点を注視点に向ける処理
	bool lookStart_ = false;         // 補間開始するか
	bool lookAlwaysTarget_ = false; // trueの間ずっと向ける
	float lookTargetLerpRate_; // フレーム補間割合
	SakuEngine::Quaternion lookToStart_;  // 補間開始時の回転
	std::optional<SakuEngine::Quaternion> lookToTarget_; // 補間目標の回転
	SakuEngine::StateTimer lookTimer_;   // 補間までの時間
	float lookTimerRate_;    // 目標時間の倍速率
	float targetXRotation_;  // 目標X回転
	std::optional<float> anyTargetXRotation_;
	float startFovY_; // 開始時
	float initFovY_;  // 目標

	float lookYawOffset_;     // Y回転オフセット
	int lookYawDirection_;    // 開始時の最短補間方向
	int preLookYawDirection_; // 開始時の最短補間方向の前回値

	// アニメーションを読み込んだか
	bool isLoadedAnim_;

	//--------- functions ----------------------------------------------------

	// update
	void UpdateLookToTarget();
	void UpdateLookAlwaysTarget();

	// helper
	SakuEngine::Quaternion GetTargetRotation() const;

	// json
	void ApplyJson();
	void SaveJson();
};
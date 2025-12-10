#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2DArray.h>
#include <Game/Objects/GameScene/Player/Structure/PlayerStructures.h>

// front
class FollowCamera;

//============================================================================
//	TargetNavigation class
//	方向を示すオブジェクト
//============================================================================
class TargetNavigation {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TargetNavigation() = default;
	~TargetNavigation() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetCamera(FollowCamera* camera) { camera_ = camera; }

	// 注視点とピボットの座標設定
	void SetTargetPos(const Vector3& targetPos) { targetPos_ = targetPos; }
	void SetPivotPos(const Vector3& pivotPos) { pivotPos_ = pivotPos; }

	// 完全に何も表示しなくさせる
	void SetDisable(bool isDisable) { isDisable_ = isDisable; }
	// 表示するか
	void SetIsDisplay(bool isDisplay);
	// 点滅させるか
	void SetIsBlink(bool isBlink) { isBlink_ = isBlink; }
	// カメラ内に注視点があるかチェックを行うか
	void SetInFrustumCheck(bool inFrustum) { inFrustumCheck_ = inFrustum; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const FollowCamera* camera_;

	// 表示するオブジェクト
	std::unique_ptr<GameObject2DArray> objectArray_;

	// パラメータ
	Vector3 targetPos_;       // 注視点の座標
	Vector2 screenTargetPos_; // 画面上の注視点の座標
	Vector3 pivotPos_;        // ピボットの座標
	Vector2 screenPivotPos_;  // 画面上のピボットの座標
	float pivotOffsetY_;      // ピボットのYオフセット

	float maxDistanceToTarget_; // 注視点までの最大距離
	float minDistanceToTarget_; // 注視点までの最小距離
	EasingType distanceEasingType_;

	float minDistanceScale_;        // 最小距離のスケール
	float waveCenter_;              // 波の中心
	float waveAmplitude_;           // 波の振幅
	StateTimer distanceScaleTimer_; // 距離スケールタイマー

	// 現在のUI角度
	float currentAngleRadian_;

	// 表示切替タイマー
	bool isDisplay_;
	StateTimer alphaTimer_;

	// 点滅
	bool isBlink_;
	Color targetBlinkColor_;
	StateTimer blinkColorTimer_;

	// カメラ内に注視点があるかチェックを行うか
	bool inFrustumCheck_;

	// 完全に何も表示しなくさせる
	bool isDisable_;

	//--------- functions ----------------------------------------------------

	// 注視点がカメラ内にあるか
	void CheckInCamera();
	// 点滅
	void UpdateBlink(float alpha);

	// json
	void ApplyJson();
	void SaveJson();
};
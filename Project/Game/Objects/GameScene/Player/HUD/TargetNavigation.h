#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2DArray.h>

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

	// 表示するか
	void IsDisplay(bool isDisplay);
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

	// 現在のUI角度
	float currentAngleRadian_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
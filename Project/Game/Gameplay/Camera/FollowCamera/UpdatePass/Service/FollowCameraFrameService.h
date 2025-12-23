#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Quaternion.h>

// front
class Player;       // 基準点
class BossEnemy;    // 注視点
class FollowCamera; // 処理対象カメラ

// ServiceClasses
class FollowCameraLookInputSmoother;
class FollowCameraReturnFov;

//============================================================================
//	FollowCameraFrameService structure
//============================================================================

// パス間で共有するデータ
struct FollowCameraContext {

	SakuEngine::Vector3 cameraTranslation; // 座標
	SakuEngine::Quaternion cameraRotation; // 回転
	float cameraFovY;                      // 画角
};
// 処理依存オブジェクト
struct FollowCameraDependencies {

	const FollowCamera* camera; // 処理対象カメラ
	const Player* player;       // 基準点
	const BossEnemy* bossEnemy; // 注視点
};
// フレーム毎の各パスが処理したデータを共有するサービス
struct FollowCameraFrameService {

	const FollowCameraLookInputSmoother* inputSmoother; // 回転入力補間
	const FollowCameraReturnFov* returnFov;             // 画角元に戻す
};
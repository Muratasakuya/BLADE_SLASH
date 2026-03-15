#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Quaternion.h>
#include <Game/Scenes/Game/GameState/GameSceneState.h>

// front
class Player;       // 基準点
class BossEnemy;    // 注視点
class FollowCamera; // 処理対象カメラ

// ServiceClasses
class FollowCameraLookInputSmoother;
class FollowCameraTargetResolver;
class FollowCameraLookRotationIntegrator;
class FollowCameraPitchClamper;
class FollowCameraZoomOffsetResolver;
class FollowCameraOffsetSmoother;
class FollowCameraReturnToFollowSmoother;
class FollowCameraCalFov;

//============================================================================
//	FollowCameraFrameService structure
//============================================================================

// 更新パスID
enum class FollowCameraUpdatePassID {

	LookInputSmoother,        // 視点入力補間
	TargetResolver,           // 追従目標位置の決定
	InterTargetSmoother,      // 追従位置の目標を補間
	LookRotationIntegrator,   // 注視回転積分
	PitchClamper,             // ピッチ角度制限
	RollStabilizer,           // ロール回転安定化
	ZoomOffsetResolver,       // ズームオフセット計算
	OffsetSmoother,           // オフセット補間
	OrbitTranslationComposer, // 最終座標構成
	CalFov,                   // デフォルト画角を計算
	ReturnFov,                // 画角元に戻す
	ActionAutoLookTarget,     // 自動で注視点を向かせる
	LockOn,                   // ロックオン
	ToFollowSmoother,         // エディター更新から追従状態に戻る補間
};

// パス間で共有するデータ
struct FollowCameraContext {

	SakuEngine::Vector3 cameraTranslation; // 座標
	SakuEngine::Quaternion cameraRotation; // 回転
	float cameraFovY;                      // 画角

	SakuEngine::Vector3 interTarget; // 追従中間目標位置
};
// 処理依存オブジェクト
struct FollowCameraDependencies {

	const FollowCamera* camera; // 処理対象カメラ
	const Player* player;       // 基準点
	const BossEnemy* bossEnemy; // 注視点
};
// フレーム毎の各パスが処理したデータを共有するサービス
struct FollowCameraFrameService {

	GameSceneState sceneState; // シーン状態

	const FollowCameraLookInputSmoother* inputSmoother;           // 視点入力補間
	const FollowCameraLookRotationIntegrator* rotationIntegrator; // 注視回転積分
	const FollowCameraPitchClamper* pitchClamper;                 // ピッチ角度制限
	const FollowCameraZoomOffsetResolver* zoomOffsetResolver;     // ズームオフセット計算
	const FollowCameraTargetResolver* targetResolver;             // 追従目標位置の決定
	const FollowCameraOffsetSmoother* offsetSmoother;             // オフセット補間
	const FollowCameraReturnToFollowSmoother* toFollowSmoother;   // エディター更新から追従状態に戻る補間
	const FollowCameraCalFov* calFov;                             // デフォルト画角を計算
};
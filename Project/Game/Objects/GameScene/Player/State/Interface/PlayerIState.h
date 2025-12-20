#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/InputMapper.h>
#include <Engine/MathLib/MathUtils.h>
#include <Engine/Object/State/StateNode.h>
#include <Game/Objects/GameScene/Player/Structure/PlayerStructures.h>
#include <Game/Objects/GameScene/Player/Input/PlayerInputAction.h>

// front
class Player;
class BossEnemy;
class FollowCamera;

//============================================================================
//	PlayerIState class
//	プレイヤー状態のインターフェース
//============================================================================
class PlayerIState :
	public SakuEngine::StateNode<PlayerState> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerIState() = default;
	virtual ~PlayerIState() = default;

	// エフェクトの生成
	virtual void CreateEffect() {}

	// 状態遷移時
	virtual void Enter() override = 0;

	// 更新処理
	virtual void Update() override = 0;

	// 常に行う更新処理
	virtual void BeginUpdateAlways() {}
	virtual void UpdateAlways() {}

	// 状態終了時
	virtual void Exit() override = 0;

	// エディター
	virtual void ImGui() = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetPlayer(Player* player) { player_ = player; }
	void SetBossEnemy(const BossEnemy* bossEnemy) { bossEnemy_ = bossEnemy; }
	void SetFollowCamera(FollowCamera* followCamera) { followCamera_ = followCamera; }

	virtual bool GetCanExit() const { return canExit_; }
	// 必要であれば継承先で回避中かどうかを返す
	virtual bool IsAvoidance() const { return false; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 状態遷移対象
	Player* player_;
	FollowCamera* followCamera_;
	const BossEnemy* bossEnemy_;

	bool canExit_ = false; // 状態終了可能か

	float nextAnimDuration_;    // 次のアニメーション遷移にかかる時間
	float rotationLerpRate_;    // 回転補間割合
	float targetCameraRotateX_; // 目標カメラX軸回転
};
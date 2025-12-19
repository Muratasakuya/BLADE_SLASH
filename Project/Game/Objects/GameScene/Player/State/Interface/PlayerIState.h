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
namespace SakuEngine {
	class PostProcessSystem;
}

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

	PlayerIState();
	virtual ~PlayerIState() = default;

	// 状態遷移時
	virtual void Enter() = 0;

	// 更新処理
	virtual void Update() override = 0;

	// 常に行う更新処理
	virtual void BeginUpdateAlways() {}
	virtual void UpdateAlways() {}
	virtual void EndUpdateAlways() {}

	// 状態終了時
	virtual void Exit() = 0;

	// imgui
	virtual void ImGui() = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetPlayer(Player* player) { player_ = player; }

	void SetInputMapper(const SakuEngine::InputMapper<PlayerInputAction>* inputMapper) { inputMapper_ = inputMapper; }
	void SetBossEnemy(const BossEnemy* bossEnemy) { bossEnemy_ = bossEnemy; }
	void SetFollowCamera(FollowCamera* followCamera) { followCamera_ = followCamera; }

	void SetCanExit(bool canExit) { canExit_ = canExit; }
	void SetPreState(PlayerState preState) { preState_ = preState; }

	virtual bool GetCanExit() const { return canExit_; }
	bool IsAvoidance() const { return isAvoidance_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 状態遷移対象
	Player* player_;

	const SakuEngine::InputMapper<PlayerInputAction>* inputMapper_;
	const BossEnemy* bossEnemy_;
	FollowCamera* followCamera_;

	SakuEngine::PostProcessSystem* postProcess_;

	// 遷移前の状態
	PlayerState preState_;

	// 共通parameters
	const float epsilon_ = std::numeric_limits<float>::epsilon();

	float nextAnimDuration_; // 次のアニメーション遷移にかかる時間
	bool canExit_ = true;    // 遷移可能かどうか
	float rotationLerpRate_; // 回転補間割合

	float targetCameraRotateX_; // 目標カメラX軸回転

	bool isAvoidance_ = false; // 回避行動中かどうか

	//--------- functions ----------------------------------------------------

	// helper
	void SetRotateToDirection(const SakuEngine::Vector3& move);

	// プレイヤー、敵の座標取得(Yを固定するため)
	SakuEngine::Vector3 GetPlayerFixedYPos() const;
	SakuEngine::Vector3 GetBossEnemyFixedYPos() const;

	// プレイヤーの敵との距離
	float GetDistanceToBossEnemy() const;

	// プレイヤーから敵への方向
	SakuEngine::Vector3 GetDirectionToBossEnemy() const;
};
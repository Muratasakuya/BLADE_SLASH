#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>

// imgui
#include <imgui.h>
// front
class BossEnemy;
class Player;
class FollowCamera;
class GameLight;
class Skybox;

//============================================================================
//	BossEnemyGreatAttackIState class
//	大技攻撃状態の各状態のインターフェース
//============================================================================
class BossEnemyGreatAttackIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackIState();
	virtual ~BossEnemyGreatAttackIState() = default;

	// 共通初期化
	void Init(BossEnemy* bossEnemy, Player* player, FollowCamera* followCamera, GameLight* gameLight);

	// 状態遷移時
	virtual void Enter() = 0;

	// 更新処理
	virtual void Update() = 0;
	virtual void UpdateAlways() {}

	// 状態終了時
	virtual void Exit() = 0;

	// imgui
	virtual void ImGui() = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	// 状態遷移可能か
	bool CanExit() const { return canExit_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Player* player_;
	FollowCamera* followCamera_;
	GameLight* gameLight_;
	BossEnemy* bossEnemy_;
	Skybox* skybox_;

	// 状態終了フラグ
	bool canExit_ = false;

	//--------- functions ----------------------------------------------------

};
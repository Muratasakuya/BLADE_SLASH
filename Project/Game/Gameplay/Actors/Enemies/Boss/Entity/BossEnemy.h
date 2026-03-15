#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

// scene
#include <Game/Scenes/Game/GameState/GameSceneState.h>
// weapon
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemyWeapon.h>
// state
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/BossEnemyStateController.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/Requests/BossEnemyRequestFalter.h>
// collision
#include <Game/Gameplay/Actors/Enemies/Boss/Collision/BossEnemyAttackCollision.h>
// animation
#include <Game/Gameplay/Actors/Enemies/Boss/Start/BossEnemyStartAnimation.h>

// front
class Player;
class GameEventBus;

//============================================================================
//	BossEnemy class
//	ボスクラス
//============================================================================
class BossEnemy :
	public SakuEngine::GameObject3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemy() = default;
	~BossEnemy() = default;

	// 初期化
	void DerivedInit() override;

	// ボスの状態更新
	void Update(GameSceneState sceneState);

	// エディター
	void DerivedImGui() override;

	/*-------- collision ----------*/

	// 衝突コールバック関数
	void OnCollisionEnter([[maybe_unused]] const SakuEngine::CollisionBody* collisionBody) override;

	/*---------- parry ------------*/

	// パリィ受付
	bool ConsumeParryTiming();
	// パリィ受付可能時間の通知
	void TellParryTiming();
	// パリィ受付時間リセット
	void ResetParryTiming() { parryTimingTickets_ = 0; }

	//--------- accessor -----------------------------------------------------

	void SetPlayer(Player* player);
	void SetFollowCamera(FollowCamera* followCamera);
	void SetEventBus(GameEventBus* bus) { eventBus_ = bus; }

	void SetAlpha(float alpha);
	void SetCastShadow(bool cast);
	void SetDecreaseToughnessProgress(float progress);
	void RequestHit();

	const BossEnemyStats& GetStats() const { return stats_; }
	BossEnemyAttackCollision* GetAttackCollision() const { return attackCollision_.get(); }

	SakuEngine::Vector3 GetWeaponTranslation() const;
	SakuEngine::Quaternion GetWeaponRotation() const;
	void SetWeaponRejection(bool isRejection) { weapon_->SetIsRejection(isRejection); }
	BossEnemyState GetCurrentState() const { return stateController_->GetCurrentState(); }

	int GetDamage() const;
	bool IsDead() const;
	bool IsInvincible() const { return  preSceneState_ == GameSceneState::Start; }
	uint32_t GetCurrentPhaseIndex() const;
	const ParryParameter& GetParryParam() const { return stateController_->GetParryParam(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const Player* player_;
	GameSceneState preSceneState_;

	// 使用する武器
	std::unique_ptr<BossEnemyWeapon> weapon_;

	// 状態の管理
	std::unique_ptr<BossEnemyStateController> stateController_;
	std::unique_ptr<BossEnemyRequestFalter> requestFalter_;
	BossEnemyState preState_;

	// 攻撃の衝突
	std::unique_ptr<BossEnemyAttackCollision>  attackCollision_;

	// イベント発行先
	GameEventBus* eventBus_ = nullptr;

	// アニメーション
	std::unique_ptr<BossEnemyStartAnimation> startAnimation_;

	// parameters
	SakuEngine::Transform3D initTransform_; // 初期化時の値
	BossEnemyStats stats_;      // ステータス
	uint32_t parryTimingTickets_ = 0;

	// editor
	int selectedPhaseIndex_;
	BossEnemyState editingState_;
	bool isDrawDistanceLevel_ = false;

	//--------- functions ----------------------------------------------------

	  // json
	void ApplyJson();
	void SaveJson();

	// init
	void InitWeapon();
	void InitAnimations();
	void InitCollision();
	void InitState();
	void InitAnimation();

	// update
	void UpdateBeginGame();
	void UpdatePlayGame();
	void UpdateEndGame();
	void CheckSceneState(GameSceneState sceneState);

	// helper
	void SetInitTransform();
	void CalDistanceToTarget();
	void DebugCommand();
};
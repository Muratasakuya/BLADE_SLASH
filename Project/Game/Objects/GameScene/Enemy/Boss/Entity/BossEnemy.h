#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

// scene
#include <Game/Scene/GameState/GameSceneState.h>
// weapon
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemyWeapon.h>
// state
#include <Game/Objects/GameScene/Enemy/Boss/State/BossEnemyStateController.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/BossEnemyRequestFalter.h>
// collision
#include <Game/Objects/GameScene/Enemy/Boss/Collision/BossEnemyAttackCollision.h>
// HUD
#include <Game/Objects/GameScene/Enemy/Boss/HUD/BossEnemyHUD.h>
// animation
#include <Game/Objects/GameScene/Enemy/Boss/Start/BossEnemyStartAnimation.h>

// front
class Player;

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

	void SetAlpha(float alpha);
	void SetCastShadow(bool cast);
	void SetDecreaseToughnessProgress(float progress);
	void RequestHit();

	const BossEnemyStats& GetStats() const { return stats_; }
	BossEnemyAttackCollision* GetAttackCollision() const { return attackCollision_.get(); }
	BossEnemyHUD* GetHUD() const { return hudSprites_.get(); }

	SakuEngine::Vector3 GetWeaponTranslation() const;
	SakuEngine::Quaternion GetWeaponRotation() const;
	void SetWeaponRejection(bool isRejection) { weapon_->SetIsRejection(isRejection); }
	BossEnemyState GetCurrentState() const { return stateController_->GetCurrentState(); }

	int GetDamage() const;
	bool IsDead() const;
	bool IsInvincible() const { return hudSprites_->IsDisable(); }
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

	// HUD
	std::unique_ptr<BossEnemyHUD> hudSprites_;

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
	void InitHUD();
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
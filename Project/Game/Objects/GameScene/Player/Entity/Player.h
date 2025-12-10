#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/EffectGroup.h>
#include <Engine/Object/Base/GameObject3D.h>

// weapon
#include <Game/Objects/GameScene/Player/Entity/PlayerWeapon.h>
// state
#include <Game/Objects/GameScene/Player/State/PlayerStateController.h>
// collision
#include <Game/Objects/GameScene/Player/Collision/PlayerAttackCollision.h>
// HUD
#include <Game/Objects/GameScene/Player/HUD/PlayerHUD.h>
#include <Game/Objects/GameScene/Player/HUD/PlayerStunHUD.h>
#include <Game/Objects/GameScene/Player/HUD/TargetNavigation.h>

// front
class SubPlayer;

//============================================================================
//	Player class
//	プレイヤー
//============================================================================
class Player :
	public SakuEngine::GameObject3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Player() = default;
	~Player() = default;

	// 初期化
	void DerivedInit() override;

	// 状態の更新
	void Update() override;

	// エディター
	void DerivedImGui() override;

	// Y座標の制限
	void ClampInitPosY();

	/*-------- collision ----------*/

	// 衝突コールバック関数
	void OnCollisionEnter(const SakuEngine::CollisionBody* collisionBody) override;

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(const BossEnemy* bossEnemy);
	void SetFollowCamera(FollowCamera* followCamera);
	void SetSubPlayer(SubPlayer* subPlayer);

	// 武器の向きを反転させる
	void SetReverseWeapon(bool isReverse, PlayerWeaponType type);
	// 武器の位置を初期化する
	void ResetWeaponTransform(PlayerWeaponType type);

	void ResetState() { stateController_->SetForcedState(*this, PlayerState::Idle); }
	PlayerState GetCurrentState() const { return stateController_->GetCurrentState(); }

	PlayerAttackCollision* GetAttackCollision() const { return attackCollision_.get(); }
	PlayerHUD* GetHUD() const { return hudSprites_.get(); }
	PlayerStunHUD* GetStunHUD() const { return stunHudSprites_.get(); }
	PlayerWeapon* GetWeapon(PlayerWeaponType type) const;
	TargetNavigation* GetTargetNavigation() const { return targetNavigation_.get(); }

	int GetDamage() const;
	int GetToughness() const { return stats_.toughness; }
	bool IsDead() const { return stats_.currentHP == 0; }
	bool IsActiveParry() const { return stateController_->IsActiveParry(); }
	const SakuEngine::Transform3D& GetInitTransform() const { return initTransform_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const BossEnemy* bossEnemy_;

	// 使用する武器
	std::unique_ptr<PlayerWeapon> rightWeapon_; // 右手
	std::unique_ptr<PlayerWeapon> leftWeapon_;  // 左手

	// 状態の管理
	std::unique_ptr<PlayerStateController> stateController_;

	// 攻撃の衝突
	std::unique_ptr<PlayerAttackCollision> attackCollision_;

	// HUD
	std::unique_ptr<PlayerHUD> hudSprites_;
	std::unique_ptr<PlayerStunHUD> stunHudSprites_;
	std::unique_ptr<TargetNavigation> targetNavigation_;

	// 回避エフェクト
	std::unique_ptr<SakuEngine::EffectGroup> avoidEffect_;

	// parameters
	SakuEngine::Transform3D initTransform_; // 初期化時の値
	PlayerStats stats_; // ステータス

	// 敵のスタン中の更新になったか
	bool isStunUpdate_;

	// ボスの状態を監視してパリィ入力示唆を出す
	std::optional<BossEnemyState> exitParryBossEnemyState_;
	bool isCanParryBossEnemy_;

	// json
	Json cacheJsonData_;

	// editor
	PlayerState editingState_;
	// フラグで攻撃を受けても食らわないようにする
	bool isInvincible_ = false;

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
	void InitEffects();

	// update
	void UpdateTargetNavigation();

	// helper
	void SetInitTransform();
	void CheckBossEnemyStun();
	void CheckBossEnemyParry();
};
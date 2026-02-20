#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Effect/User/EffectGroup.h>
#include <Engine/Object/Base/GameObject3D.h>

// weapon
#include <Game/Gameplay/Actors/Player/Entity/PlayerWeapon.h>
// state
#include <Game/Gameplay/Actors/Player/StateMachine/PlayerStateController.h>
// collision
#include <Game/Gameplay/Actors/Player/Collision/PlayerAttackCollision.h>
// editor
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/PlayerComboActionEditor.h>

// front
class SubPlayer;
class GameEventBus;

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

	/*-------- collision ----------*/

	// 衝突コールバック関数
	void OnCollisionEnter(const SakuEngine::CollisionBody* collisionBody) override;

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(BossEnemy* bossEnemy);
	void SetFollowCamera(FollowCamera* followCamera);
	void SetEventBus(GameEventBus* bus) { eventBus_ = bus; }

	// 武器の向きを反転させる
	void SetReverseWeapon(bool isReverse, PlayerWeaponType type);
	// 武器の位置を初期化する
	void ResetWeaponTransform(PlayerWeaponType type);

	void ResetState() { stateController_->SetForcedState(PlayerState::Idle); }
	PlayerState GetCurrentState() const { return stateController_->GetCurrentState(); }

	const PlayerStats& GetStats() const { return stats_; }
	PlayerAttackCollision* GetAttackCollision() const { return attackCollision_.get(); }
	PlayerWeapon* GetWeapon(PlayerWeaponType type) const;

	int GetDamage() const;
	int GetToughness() const { return stats_.toughness; }
	bool IsDead() const { return stats_.currentHP == 0; }
	bool IsActiveParry() const { return stateController_->IsActiveParry(); }
	const SakuEngine::Transform3D& GetInitTransform() const { return initTransform_; }

	// アニメーション名一覧の取得
	const std::vector<std::string>& GetAnimNames() const { return animNames_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const BossEnemy* bossEnemy_;

	// アニメーション名一覧
	std::vector<std::string> animNames_;

	// コンボアクションエディター
	std::unique_ptr<PlayerComboActionEditor> comboActionEditor_;

	// 使用する武器
	std::unique_ptr<PlayerWeapon> rightWeapon_; // 右手
	std::unique_ptr<PlayerWeapon> leftWeapon_;  // 左手

	// 状態の管理
	std::unique_ptr<PlayerStateController> stateController_;

	// 攻撃の衝突
	std::unique_ptr<PlayerAttackCollision> attackCollision_;

	// イベント発行先
	GameEventBus* eventBus_ = nullptr;

	// parameters
	SakuEngine::Transform3D initTransform_; // 初期化時の値
	PlayerStats stats_; // ステータス
	SakuEngine::StateTimer recoverSkilPointTimer_; // スキルポイント回復タイマー

	// 敵のスタン中の更新になったか
	bool isStunUpdate_;

	// json
	Json cacheJsonData_;

	// デバッグ
	PlayerState editingState_;
	// フラグで攻撃を受けても食らわないようにする
	bool isInvincible_ = false;
	// 状態マシンで動かすか、今作ってる状態で動かすか
	bool isExecuteStateMachine_ = true;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitWeapon();
	void InitAnimations();
	void InitCollision();
	void InitState();

	// update
	void UpdateSKilPoint();

	// helper
	void CheckExecutedParry();
	void SetInitTransform();
	void ClampInitPosY();
};
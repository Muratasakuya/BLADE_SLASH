#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/EffectGroup.h>
#include <Engine/Collision/Collider.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyProjectileAttackState class
//	弾を使って攻撃する状態
//============================================================================
class BossEnemyProjectileAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyProjectileAttackState(uint32_t phaseCount);
	~BossEnemyProjectileAttackState() = default;

	void CreateEffect() override;

	void Enter() override;

	void Update() override;
	void UpdateAlways() override;

	void Exit() override;

	// imgui
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		Launch, // 発生装置起動
		Attack, // 順に攻撃
	};

	// 弾の衝突情報
	struct BulletCollision {

		std::unique_ptr<SakuEngine::Collider> collider; // 衝突判定

		bool isActive;         // 有効フラグ
		SakuEngine::Vector3 startPos;      // 発生位置
		SakuEngine::Vector3 targetPos;     // 目標位置
		SakuEngine::StateTimer moveTimer;  // 座標移動タイマー
		SakuEngine::Transform3D transform; // 弾の仮Transform

		// 絶対に当たらない座標
		const SakuEngine::Vector3 collisionSafePos_ = SakuEngine::Vector3(0.0f, -128.0f, 0.0f);

		// 初期化
		void Init();

		// 更新
		void Update(float duration);

		// 位置補間
		void LerpTranslation(float duration);
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;
	uint32_t phaseCount_; // フェーズの数

	// フェーズに応じた弾を飛ばす数、奇数個
	std::vector<uint32_t> phaseBulletCounts_;
	// 現在のフェーズインデックス、Enterで取得して変更しない
	uint32_t currentPhaseIndex_;
	// 発生させる順番のインデックス
	std::vector<int32_t> launchIndices_;

	// 発生しきる時間、0.0f~1.0fの間に等間隔に発生
	SakuEngine::StateTimer launchTimer_;
	// 攻撃が終了するまでの時間、弾の数に応じて変える
	// 等間隔で弾を発生させる
	float bulletAttackDuration_;
	SakuEngine::StateTimer attackTimer_;

	// 真ん中の発生位置のY座標
	float launchTopPosY_;
	// 左右の発生位置のオフセット
	SakuEngine::Vector3 launchOffsetPos_;

	// 実際に発生させる位置
	std::vector<SakuEngine::Vector3> launchPositions_;
	// 目標位置からのオフセット
	float targetDistance_;

	// 発生起動エフェクト
	std::unique_ptr<SakuEngine::EffectGroup> launchEffect_;
	// 弾
	static const uint32_t kMinBulletCount_ = 3;
	static const uint32_t kMaxBulletCount_ = 5;
	std::array<std::unique_ptr<SakuEngine::EffectGroup>, kMaxBulletCount_> bulletEffects_;
	const std::string bulletParticleNodeKey_ = "bossAttackProjectile";
	// 弾の衝突判定
	std::array<BulletCollision, kMaxBulletCount_> bulletColliders_;
	// 判定の半径
	float bulletCollisionRadius_;
	// 弾の補間速度
	float bulletLerpDuration_;

	// エフェクト発生済みフラグ
	std::vector<bool> launchEmited_;
	std::vector<bool> bulletEmited_;

	// エディター
	bool isEditMode_ = false;
	int32_t editingPhase_; // 編集中のフェーズ
	SakuEngine::Vector3 editStartPos_; // デバッグ用開始位置
	SakuEngine::Vector3 editEndPos_;   // デバッグ用終了位置

	//--------- functions ----------------------------------------------------

	// update
	void UpdateLaunch();
	void UpdateAttack();

	// helper
	void BeginLaunchPhase();
	// 発生順序のインデックスを設定
	void SetLeftToRightIndices();
	// 発生位置を設定
	void SetLaunchPositions(int32_t phaseIndex);
};
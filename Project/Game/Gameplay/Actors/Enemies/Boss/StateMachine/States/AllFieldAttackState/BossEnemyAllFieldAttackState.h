#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/Interface/BossEnemyIState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/AllFieldAttackState/Item/BossEnemyProtrusion.h>

// c++
#include <array>

//============================================================================
//	BossEnemyAllFieldAttackState class
//	フィールド攻撃状態
//============================================================================
class BossEnemyAllFieldAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyAllFieldAttackState();
	~BossEnemyAllFieldAttackState() = default;

	void Enter() override;

	void Update() override;
	void UpdateAlways() override;

	void Exit() override;

	void ImGui() override;

	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		Previous,   // 突起攻撃準備
		Protrusion, // 突起攻撃
		Area,       // 範囲攻撃
		Finish,     // 終了
	};
	// 範囲攻撃の状態
	enum class AreaAttackState {

		Charge, // 範囲攻撃のチャージ
		Emit,   // 範囲攻撃の発生
		Wait,   // 範囲攻撃中
	};

	// 範囲攻撃のコライダー情報
	struct AreaAttackCollider {

		// コライダー実体
		std::unique_ptr<SakuEngine::Collider> collider;
		SakuEngine::Transform3D transform;
		// 移動速度
		SakuEngine::Vector3 velocity;

		// ライフタイム
		SakuEngine::StateTimer lifeTimer;

		// 生存フラグ
		bool isAlive = true;
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_ = State::Previous;

	//---------- Previous ----------------------------------------------------

	// 中心までの座標移動
	SakuEngine::SimpleAnimation<SakuEngine::Vector3> moveToCenterAnim_;

	// 移動中のエフェクト
	std::unique_ptr<SakuEngine::EffectGroup> moveToCenterEffect_;

	//--------- Protrusion ---------------------------------------------------

	// 突起発生範囲半径の最小(プレイヤーがどれだけ中心から離れているか)
	float protrusionEmitRadiusMin_ = 0.0f;
	float playerDistanceToCenter_ = 0.0f;
	// 突起発生個数
	static const uint32_t maxProtrusionCount_ = 4;

	// 突起
	std::array<std::unique_ptr<BossEnemyProtrusion>, maxProtrusionCount_> protrusions_;

	// 突起の残留時間
	float protrusionLifeTime_ = 0.0f;

	//--------- AreaAttack ---------------------------------------------------

	AreaAttackState areaAttackState_ = AreaAttackState::Charge;

	// 範囲攻撃のチャージ時間
	SakuEngine::StateTimer areaAttackChargeTimer_;
	// 範囲攻撃のエフェクト
	std::unique_ptr<SakuEngine::EffectGroup> areaAttackEffect_;
	// 範囲攻撃時間
	SakuEngine::StateTimer areaAttackTimer_;

	// 範囲攻撃のダメージ判定用コライダー
	std::list<AreaAttackCollider> areaAttackColliders_;
	// 範囲攻撃のコライダーの半径
	float areaAttackColliderRadius_ = 4.0f;
	// 壁に当たらない場合のライフタイム
	float longColliderLifeTime_ = 0.0f;
	// 壁に当たる場合のライフタイム
	float shortColliderLifeTime_ = 0.0f;
	// 球を出す間隔角度
	float emitAngleInterval_ = 15.0f;
	// 球の速度
	float areaAttackColliderSpeed_ = 0.0f;
	// 球を出す間隔
	SakuEngine::StateTimer areaAttackEmitIntervalTimer_;

	//--------- functions ----------------------------------------------------

	// 突起攻撃準備
	void PrepareProtrusionAttack();
	// 突起発生更新
	void UpdateProtrusionEmit();
	void EmitAll();
	// 範囲攻撃の発生処理
	void UpdateAreaAttack();
	void CreateAreaAttackCollider(bool isLong, const SakuEngine::Vector3& direction);
	void EmitAreaAttackCollider();
};
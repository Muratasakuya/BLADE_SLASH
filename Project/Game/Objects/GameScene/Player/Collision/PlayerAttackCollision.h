#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/Collider.h>
#include <Engine/Effect/User/GameEffect.h>
#include <Game/Objects/GameScene/Player/Structure/PlayerStructures.h>

// front
class BossEnemy;

//============================================================================
//	PlayerAttackCollision class
//	プレイヤーの状態に応じた攻撃判定、ヒットストップ処理
//============================================================================
class PlayerAttackCollision :
	public SakuEngine::Collider {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAttackCollision() = default;
	~PlayerAttackCollision() = default;

	// 初期化
	void Init();

	// 衝突判定更新
	void Update(const SakuEngine::Transform3D& transform);

	// エディター
	void ImGui();

	// json
	void ApplyJson(const Json& data);
	void SaveJson(Json& data);

	// 衝突コールバック関数
	void OnCollisionEnter(const SakuEngine::CollisionBody* collisionBody) override;

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(const BossEnemy* bossEnemy) { bossEnemy_ = bossEnemy; }

	// プレイヤーの状態に応じた攻撃判定に切り替え
	void SetEnterState(PlayerState state);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 判定区間
	struct TimeWindow {

		float on;  // 衝突開始時間(判定入り)
		float off; // 衝突終了時間(判定を消す)
	};

	struct AttackParameter {

		SakuEngine::Vector3 centerOffset; // player座標からのオフセット
		SakuEngine::Vector3 size;         // サイズ
		float hitInterval;    // 多段ヒット
		std::vector<TimeWindow> windows;

		// ヒットストップ
		float waitTime;
		float lerpSpeed;
		float timeScale;
		EasingType timeScaleEasing;

		// 衝突判定の基準を敵の位置にするか
		bool isEnemyBased = false;
	};

	//--------- variables ----------------------------------------------------

	const BossEnemy* bossEnemy_;

	SakuEngine::CollisionBody* weaponBody_;
	const SakuEngine::Transform3D* transform_;

	std::unordered_map<PlayerState, AttackParameter> table_; // 状態毎の衝突
	const AttackParameter* currentParameter_;                // 現在の状態の値

	float currentTimer_; // 現在の経過時間、全部共通
	float reHitTimer_;   // 多段ヒット経過時間

	// editor
	PlayerState editingState_;

	//--------- functions ----------------------------------------------------

	// helper
	void EditWindowParameter(const std::string& label, std::vector<TimeWindow>& windows);
};
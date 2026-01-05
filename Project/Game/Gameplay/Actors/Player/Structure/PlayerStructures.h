#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	PlayerStructures
//============================================================================

// 状態の種類
enum class PlayerState {

	None,          // 無状態、ここに状態は作成しない
	Idle,          // 何もしない
	Walk,          // 歩き...          WASD/左スティック入力
	Dash,          // ダッシュ(回避)... 移動中に右クリック/Aボタン
	Avoid,         // 回避...          静止中OR攻撃中に右クリック/Aボタン
	Attack_1st,    // 通常攻撃1段目...  左クリック/Xボタン
	Attack_2nd,    // 通常攻撃2段目...  左クリック/Xボタン(1段目攻撃中にのみ入力受付)
	Attack_3rd,    // 通常攻撃3段目...  左クリック/Xボタン(2段目攻撃中にのみ入力受付)
	Attack_4th,    // 通常攻撃4段目...  左クリック/Xボタン(3段目攻撃中にのみ入力受付)
	SkilAttack,    // スキル攻撃...     E/Yボタン
	ParryWait,     // カウンター待機...  Space/ショルダーボタン
	Parry,         // 攻撃カウンター...  左クリック/Xボタン
	Falter,        // 攻撃を受けた時の怯み
	Count,
};

// 武器の種類
enum class PlayerWeaponType {

	Left,
	Right
};

// ステータス
struct PlayerStats {

	int32_t maxHP;     // 最大HP
	int32_t currentHP; // 現在のHP

	int32_t maxSkillPoint;      // 最大スキルポイント
	int32_t currentSkillPoint;  // 現在のスキルポイント
	int32_t skillCost;           // スキル攻撃の消費スキルポイント
	int32_t incrementSkillPoint; // スキルポイントの回復量

	std::unordered_map<PlayerState, int32_t> damages; // 各攻撃のダメージ量
	int32_t damageRandomRange;                        // ダメージのランダム範囲

	int32_t toughness; // 攻撃した時に敵に入る靭性ダメージ量
};

// 遷移条件
struct PlayerStateCondition {

	float coolTime;                           // 次に入るまでのクールタイム
	std::vector<PlayerState> allowedPreState; // 遷移元を制限
	float chainInputTime;                     // 受付猶予
	std::vector<PlayerState> interruptableBy; // 強制キャンセルできる遷移相手

	bool isArmor = false;                     // 攻撃を受けても怯まない
	bool enableInARowForceState = false;      // 連続で強制状態に入ることを許可する

	void FromJson(const Json& data);
	void ToJson(Json& data);

	// helper
	bool CheckInterruptableByState(PlayerState current);
};
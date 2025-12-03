#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/EffectGroup.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/Interface/BossEnemyGreatAttackIState.h>

//============================================================================
//	BossEnemyGreatAttackState class
//	ボスの大技攻撃処理
//============================================================================
class BossEnemyGreatAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackState();
	~BossEnemyGreatAttackState() = default;

	void InitState(BossEnemy& bossEnemy);

	void Enter(BossEnemy& bossEnemy) override;

	void Update(BossEnemy& bossEnemy) override;
	void UpdateAlways(BossEnemy& bossEnemy) override;

	void Exit(BossEnemy& bossEnemy) override;

	// imgui
	void ImGui(const BossEnemy& bossEnemy) override;

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

		Charge,  // チャージ
		Execute, // 攻撃中
		Finish   // 終了
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 状態マップ
	std::unordered_map<State, std::unique_ptr<BossEnemyGreatAttackIState>> states_;

	// 値操作する状態
	State editState_;

	// エフェクト
	// 雷攻撃(警告も)
	std::unique_ptr<EffectGroup> lightningAttackEffect_;

	//--------- functions ----------------------------------------------------

	// helper
	std::optional<State> GetNextState(State state) const;
};
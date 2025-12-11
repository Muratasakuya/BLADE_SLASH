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

	// エフェクトの発生と停止
	void StartEffects();
	void StopEffects();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		ApproachAttack,   // 近接攻撃
		InOutAreaAttack,  // 内外攻撃
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 状態マップ
	std::unordered_map<State, std::unique_ptr<BossEnemyGreatAttackIState>> states_;

	// 値操作する状態
	State editState_;

	// エフェクト
	// オーラエフェクト、チャージ終了後から
	std::unique_ptr<SakuEngine::EffectGroup> bossAuraEffect_;
	// ボスの残像エフェクト
	std::unique_ptr<SakuEngine::EffectGroup> bossAfterImageEffect_;
	std::unique_ptr<SakuEngine::EffectGroup> bossWeaponAfterImageEffect_;
	bool isEmitAuraEffect_ = false;
	// 雷攻撃(警告も)
	std::unique_ptr<SakuEngine::EffectGroup> lightningAttackEffect_;

	//--------- functions ----------------------------------------------------

	// helper
	std::optional<State> GetNextState(State state) const;

	// エフェクトの発生
	void EmitEffect(State state);
};
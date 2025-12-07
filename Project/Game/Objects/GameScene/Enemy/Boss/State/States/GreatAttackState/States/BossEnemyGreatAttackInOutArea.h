#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/EffectGroup.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/Interface/BossEnemyGreatAttackIState.h>

//============================================================================
//	BossEnemyGreatAttackInOutArea class
//============================================================================
class BossEnemyGreatAttackInOutArea :
	public BossEnemyGreatAttackIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackInOutArea();
	~BossEnemyGreatAttackInOutArea() = default;

	// 状態遷移時
	void Enter() override;

	// 更新処理
	void Update() override;
	void UpdateAlways() override;

	// 状態終了時
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

		Out, // 外側
		In   // 内側
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 雷攻撃の発生
	float outAreaRadius_;     // 外側の範囲半径
	uint32_t lightningCount_; // 雷攻撃の個数(半径の分割数)
	static const uint32_t maxLightningCount_ = 32u;

	// 雷攻撃(警告も)
	std::array<std::unique_ptr<EffectGroup>, maxLightningCount_> lightningAttackEffects_;

	//--------- functions ----------------------------------------------------

	// 状態毎の更新
	void UpdateOut();
	void UpdateIn();

	// 雷攻撃発生
	void EmitLightningAttack();
};
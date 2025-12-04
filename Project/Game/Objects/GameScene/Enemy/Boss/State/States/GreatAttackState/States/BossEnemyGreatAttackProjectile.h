#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/Interface/BossEnemyGreatAttackIState.h>

//============================================================================
//	BossEnemyGreatAttackProjectile class
//============================================================================
class BossEnemyGreatAttackProjectile :
	public BossEnemyGreatAttackIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackProjectile() = default;
	~BossEnemyGreatAttackProjectile() = default;

	// 状態遷移時
	void Enter() override;

	// 更新処理
	void Update() override;

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

	//--------- variables ----------------------------------------------------



	//--------- functions ----------------------------------------------------

};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/GuardCondition/Interface/IPlayerGuardCondition.h>

//============================================================================
//	PlayerGuardOverSkill class
//	プレイヤーのスキルゲージが一定以上かどうか
//============================================================================
class PlayerGuardOverSkill :
	public IPlayerGuardCondition {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerGuardOverSkill() = default;
	~PlayerGuardOverSkill() = default;

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 条件結果
	bool GetResult() const override;
};
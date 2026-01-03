#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/GuardCondition/Interface/IPlayerGuardCondition.h>

//============================================================================
//	PlayerGuardConditionFactory class
//	プレイヤーガード条件の生成を行うファクトリークラス
//============================================================================
class PlayerGuardConditionFactory {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerGuardConditionFactory() = default;
	~PlayerGuardConditionFactory() = default;

	// ガード条件生成
	static std::unique_ptr<IPlayerGuardCondition> CreateCondition(PlayerGuardConditionType type);
};
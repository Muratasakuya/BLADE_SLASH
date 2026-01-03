#include "PlayerGuardConditionFactory.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/GuardCondition/Conditions/PlayerGuardOverSkill.h>

//============================================================================
//	PlayerGuardConditionFactory classMethods
//============================================================================

std::unique_ptr<IPlayerGuardCondition> PlayerGuardConditionFactory::CreateCondition(
	PlayerGuardConditionType type) {

	switch (type) {
	case PlayerGuardConditionType::OverSkillGauge:

		return std::make_unique<PlayerGuardOverSkill>();
	}
	return nullptr;
}
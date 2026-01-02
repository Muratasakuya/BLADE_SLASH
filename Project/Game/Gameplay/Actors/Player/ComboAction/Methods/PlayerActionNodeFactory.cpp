#include "PlayerActionNodeFactory.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerMoveFrontAttack.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerMoveWaysAttack.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerThrowWeaponAttack.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerCatchWeaponAttack.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerThrustAttack.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerJumpAttack.h>

//============================================================================
//	PlayerActionNodeFactory classMethods
//============================================================================

std::unique_ptr<PlayerIActionNode> PlayerActionNodeFactory::CreateNode(PlayerActionNodeType type) {

	switch (type) {
	case PlayerActionNodeType::MoveFrontAttack:

		return std::make_unique<PlayerMoveFrontAttack>();
	case PlayerActionNodeType::MoveWaysAttack:

		return std::make_unique<PlayerMoveWaysAttack>();
	case PlayerActionNodeType::ThrowWeaponAttack:

		return std::make_unique<PlayerThrowWeaponAttack>();
	case PlayerActionNodeType::CatchWeaponAttack:

		return std::make_unique<PlayerCatchWeaponAttack>();
	case PlayerActionNodeType::ThrustAttack:

		return std::make_unique<PlayerThrustAttack>();
	case PlayerActionNodeType::JumpAttack:

		return std::make_unique<PlayerJumpAttack>();
	}
	return nullptr;
}
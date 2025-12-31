#include "PlayerActionNodeFactory.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerMoveFrontAttack.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerMoveWaysAttack.h>

//============================================================================
//	PlayerActionNodeFactory classMethods
//============================================================================

std::unique_ptr<PlayerIActionNode> PlayerActionNodeFactory::CreateNode(PlayerActionNodeType type) {

	switch (type) {
	case PlayerActionNodeType::MoveFrontAttack:

		return std::make_unique<PlayerMoveFrontAttack>();
	case PlayerActionNodeType::MoveWaysAttack:

		return std::make_unique<PlayerMoveWaysAttack>();
	}
	return nullptr;
}
#include "PlayerActionNodeFactory.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Nodes/PlayerMoveFrontAttack.h>

//============================================================================
//	PlayerActionNodeFactory classMethods
//============================================================================

std::unique_ptr<PlayerIActionNode> PlayerActionNodeFactory::CreateNode(PlayerActionNodeType type) {

	switch (type) {
	case PlayerActionNodeType::MoveFrontAttack:

		return std::make_unique<PlayerMoveFrontAttack>();
	}
	return nullptr;
}
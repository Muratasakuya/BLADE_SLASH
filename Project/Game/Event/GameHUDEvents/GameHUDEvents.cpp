#include "GameHUDEvents.h"

//============================================================================
//	GameHUDEvents classMethods
//============================================================================

GameHUDEvents::EntityId GameHUDEvents::MakeEntityId(const void* ptr) noexcept {

	return static_cast<EntityId>(reinterpret_cast<std::uintptr_t>(ptr));
}
#include "SpriteBufferSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPoolManager.h>
#include <Engine/Object/Data/Transform/Transform.h>
#include <Engine/Object/Data/Material/Material.h>

//============================================================================
//	SpriteBufferSystem classMethods
//============================================================================

Archetype SpriteBufferSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<Transform2D>());
	arch.set(ObjectPoolManager::GetTypeID<Sprite>());
	return arch;
}

void SpriteBufferSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* transform = ObjectPoolManager.GetData<Transform2D>(object);
		auto* sprite = ObjectPoolManager.GetData<Sprite>(object);

		// spriteの更新処理
		sprite->UpdateVertex(*transform);
	}
}

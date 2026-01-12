#include "AnimationSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPoolManager.h>
#include <Engine/Object/Data/Skinned/SkinnedAnimation.h>

//============================================================================
//	AnimationSystem classMethods
//============================================================================

Archetype AnimationSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<SkinnedAnimation>());
	return arch;
}

void AnimationSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* animation = ObjectPoolManager.GetData<SkinnedAnimation>(object);
		auto* transform = ObjectPoolManager.GetData<Transform3D>(object);
		animation->Update(transform->matrix.world);
	}
}
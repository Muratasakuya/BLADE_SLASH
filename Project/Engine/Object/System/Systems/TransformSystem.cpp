#include "TransformSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPoolManager.h>
#include <Engine/Object/Data/Transform/Transform.h>
#include <Engine/Object/Data/Text/MSDFText.h>

//============================================================================
//	TransformSystem3D classMethods
//============================================================================

Archetype Transform3DSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<Transform3D>());
	return arch;
}

void Transform3DSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* transform = ObjectPoolManager.GetData<Transform3D>(object);
		transform->UpdateMatrix();
	}
}

//============================================================================
//	Transform2DSystem classMethods
//============================================================================

Archetype Transform2DSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<Transform2D>());
	return arch;
}

void Transform2DSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* transform = ObjectPoolManager.GetData<Transform2D>(object);
		transform->UpdateMatrix();
	}
}

//============================================================================
//	TextTransform2DSystem classMethods
//============================================================================

Archetype TextTransform2DSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<TextTransform2D>());
	arch.set(ObjectPoolManager::GetTypeID<MSDFText>());
	return arch;
}

void TextTransform2DSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* transform = ObjectPoolManager.GetData<TextTransform2D>(object);
		auto* text = ObjectPoolManager.GetData<MSDFText>(object);
		transform->UpdateAllMatrix(*text);
	}
}
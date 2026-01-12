#include "MSDFTextBufferSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPoolManager.h>
#include <Engine/Object/Data/Transform/Transform.h>

//============================================================================
//	MSDFTextBufferSystem classMethods
//============================================================================

Archetype MSDFTextBufferSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<Transform2D>());
	arch.set(ObjectPoolManager::GetTypeID<MSDFTextMaterial>());
	arch.set(ObjectPoolManager::GetTypeID<MSDFText>());
	return arch;
}

void MSDFTextBufferSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	// データクリア
	textData_.clear();
	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* transform = ObjectPoolManager.GetData<Transform2D>(object);
		auto* material = ObjectPoolManager.GetData<MSDFTextMaterial>(object);
		auto* text = ObjectPoolManager.GetData<MSDFText>(object);
		// テキストの更新処理
		text->UpdateVertex(*transform);

		// 追加
		textData_.emplace_back(MSDFTextData{ transform, material, text });
	}
}
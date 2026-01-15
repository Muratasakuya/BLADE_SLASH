#include "CanvasBufferSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPoolManager.h>
#include <Engine/Object/Data/Text/MSDFText.h>
#include <Engine/Object/Data/Sprite/Sprite.h>

//============================================================================
//	CanvasBufferSystem classMethods
//============================================================================

void CanvasBufferSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	// データクリア
	canvasDataMap_.clear();

	// MSDFTextデータを収集
	{
		Archetype arch{};
		const auto& view = ObjectPoolManager.View(arch.set(ObjectPoolManager::GetTypeID<MSDFText>()));
		for (const auto& object : view) {

			auto* text = ObjectPoolManager.GetData<MSDFText>(object);
			canvasDataMap_[text->GetLayer()].emplace_back(text);
		}
	}
	// Spriteデータを収集
	{
		Archetype arch{};
		const auto& view = ObjectPoolManager.View(arch.set(ObjectPoolManager::GetTypeID<Sprite>()));
		for (const auto& object : view) {

			auto* sprite = ObjectPoolManager.GetData<Sprite>(object);
			canvasDataMap_[sprite->GetLayer()].emplace_back(sprite);
		}
	}
	// 描画順インデックスでソートを行う
	for (auto& [phase, vector] : canvasDataMap_) {
		std::stable_sort(vector.begin(), vector.end(),
			[](const BaseCanvas* dataA, const BaseCanvas* dataB) {

				const auto& canvasA = dataA->GetLayerIndex();
				const auto& canvasB = dataB->GetLayerIndex();
				// レイヤーでソート
				if (canvasA != canvasB) {
					return canvasA < canvasB;
				}
				// ブレンドモードでソート
				return static_cast<int32_t>(dataA->GetBlendMode()) < static_cast<int32_t>(dataB->GetBlendMode());
			});
	}
}
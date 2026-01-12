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

const MSDFFont* MSDFTextBufferSystem::GetMSDFFont(Asset* asset,
	const std::string& atlasTextureName, const std::string& fontJsonPath) {

	const std::string cacheKey = atlasTextureName + "|" + fontJsonPath;

	auto found = fontCache_.find(cacheKey);
	if (found != fontCache_.end()) {
		return found->second.get();
	}

	// キャッシュに無ければ新規作成して登録
	std::unique_ptr<MSDFFont> font = std::make_unique<MSDFFont>();
	font->Init(asset, atlasTextureName, fontJsonPath);

	// 登録
	const MSDFFont* fontPointer = font.get();
	fontCache_.emplace(cacheKey, std::move(font));
	return fontPointer;
}

Archetype MSDFTextBufferSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<TextTransform2D>());
	arch.set(ObjectPoolManager::GetTypeID<MSDFTextMaterial>());
	arch.set(ObjectPoolManager::GetTypeID<MSDFText>());
	return arch;
}

void MSDFTextBufferSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	// データクリア
	textData_.clear();
	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* transform = ObjectPoolManager.GetData<TextTransform2D>(object);
		auto* material = ObjectPoolManager.GetData<MSDFTextMaterial>(object);
		auto* text = ObjectPoolManager.GetData<MSDFText>(object);
		// テキストの更新処理
		text->UpdateVertex(*transform);

		// 追加
		textData_.emplace_back(MSDFTextData{ transform, material, text });
	}
}
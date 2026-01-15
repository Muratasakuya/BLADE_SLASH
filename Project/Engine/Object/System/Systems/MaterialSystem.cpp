#include "MaterialSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Object/Core/ObjectPoolManager.h>

//============================================================================
//	MaterialSystem classMethods
//============================================================================

void MaterialSystem::Init(std::vector<Material>& materials,
	const ModelData& modelData, Asset* asset) {

	// mesh数に合わせる
	materials.resize(modelData.meshes.size());
	for (uint32_t meshIndex = 0; meshIndex < modelData.meshes.size(); ++meshIndex) {

		materials[meshIndex].Init(asset);
		materials[meshIndex].SetTextureName(modelData.meshes[meshIndex].textureName.value_or("white"));

		// normalMap用のTextureがあれば設定する
		if (modelData.meshes[meshIndex].normalMapTexture.has_value()) {

			materials[meshIndex].SetNormalMapTextureName(modelData.meshes[meshIndex].normalMapTexture.value());
			materials[meshIndex].enableNormalMap = true;
		}

		// baseColorがあれば色を設定する
		if (modelData.meshes[meshIndex].baseColor.has_value()) {

			materials[meshIndex].color = modelData.meshes[meshIndex].baseColor.value();
			materials[meshIndex].emissionColor = Vector3(
				modelData.meshes[meshIndex].baseColor.value().r,
				modelData.meshes[meshIndex].baseColor.value().g,
				modelData.meshes[meshIndex].baseColor.value().b);
		}
	}
}

Archetype MaterialSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<Material>());
	return arch;
}

void MaterialSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* materials = ObjectPoolManager.GetData<Material, true>(object);
		for (auto& material : *materials) {

			material.UpdateUVTransform();
		}
	}
}

//============================================================================
//	SpriteMaterialSystem classMethods
//============================================================================

Archetype SpriteMaterialSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<SpriteMaterial>());
	return arch;
}

void SpriteMaterialSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	for (uint32_t object : ObjectPoolManager.View(Signature())) {

		auto* material = ObjectPoolManager.GetData<SpriteMaterial>(object);
		material->UpdateUVTransform();
	}
}

//============================================================================
//	TextMaterialSystem classMethods
//============================================================================

Archetype TextMaterialSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<MSDFTextMaterial>());
	return arch;
}

void TextMaterialSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	const auto& view = ObjectPoolManager.View(Signature());
	for (const auto& object : view) {

		auto* material = ObjectPoolManager.GetData<MSDFTextMaterial>(object);
		material->UpdateBuffer();
	}
}
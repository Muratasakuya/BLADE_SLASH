#include "SkyboxRenderSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPoolManager.h>

//============================================================================
//	SkyboxRenderSystem classMethods
//============================================================================

Archetype SkyboxRenderSystem::Signature() const {

	Archetype arch{};
	arch.set(ObjectPoolManager::GetTypeID<Skybox>());
	return arch;
}

void SkyboxRenderSystem::Update(ObjectPoolManager& ObjectPoolManager) {

	const auto& view = ObjectPoolManager.View(Signature());

	for (const auto& object : view) {

		auto* skybox = ObjectPoolManager.GetData<Skybox>(object);

		// skyboxの更新
		skybox->Update();
		data_ = skybox;
		isCreated_ = true;
	}
	// 存在していないので未作成として設定
	if (view.empty()) {

		isCreated_ = false;
	}
}

void SkyboxRenderSystem::Render(ID3D12GraphicsCommandList* commandList) {

	// 作成されていない場合は処理しない
	if (!isCreated_) {
		return;
	}

	// buffer設定
	// vertex
	commandList->IASetVertexBuffers(0, 1, &data_->GetVertexBuffer().GetVertexBufferView());
	commandList->IASetIndexBuffer(&data_->GetIndexBuffer().GetIndexBufferView());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// matrix
	commandList->SetGraphicsRootConstantBufferView(0, data_->GetMatrixBuffer().GetResource()->GetGPUVirtualAddress());
	// material
	commandList->SetGraphicsRootConstantBufferView(3, data_->GetMaterialBuffer().GetResource()->GetGPUVirtualAddress());

	// 描画
	commandList->DrawIndexedInstanced(data_->GetIndexCount(), 1, 0, 0, 0);
}

uint32_t SkyboxRenderSystem::GetTextureIndex() const {

	if (!isCreated_) {
		assert(false);
		return 0;
	}

	return data_->GetTextureIndex();
}

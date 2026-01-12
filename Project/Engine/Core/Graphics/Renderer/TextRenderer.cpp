#include "TextRenderer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/GPUObject/SceneConstBuffer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/Data/Material/Material.h>
#include <Engine/Object/System/Systems/MSDFTextBufferSystem.h>

//============================================================================
//	TextRenderer classMethods
//============================================================================

void TextRenderer::Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor,
	DxShaderCompiler* shaderCompiler) {

	// パイプライン作成
	pipeline_ = std::make_unique<PipelineState>();
	pipeline_->Create("msdfText.json", device, srvDescriptor, shaderCompiler);
}

void TextRenderer::Rendering(SceneConstBuffer* sceneBuffer, DxCommand* dxCommand) {

	const auto& system = ObjectManager::GetInstance()->GetSystem<MSDFTextBufferSystem>();
	const auto& textData = system->GetTextData();
	if (textData.empty()) {
		return;
	}

	// commandList取得
	ID3D12GraphicsCommandList* commandList = dxCommand->GetCommandList();

	// パイプライン設定
	commandList->SetGraphicsRootSignature(pipeline_->GetRootSignature());
	commandList->SetPipelineState(pipeline_->GetGraphicsPipeline());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// カメラの正射影行列設定
	sceneBuffer->SetOrthoProCommand(commandList, 1);

	for (const auto& [transform, material, text] : textData) {

		// 頂点バッファ設定
		commandList->IASetVertexBuffers(0, 1, &text->GetVertexBuffer().GetVertexBufferView());
		// インデックスバッファ設定
		commandList->IASetIndexBuffer(&text->GetIndexBuffer().GetIndexBufferView());

		// トランスフォーム設定
		commandList->SetGraphicsRootConstantBufferView(0, transform->GetBuffer().GetResource()->GetGPUVirtualAddress());
		// マテリアル設定
		// テクスチャ
		commandList->SetGraphicsRootDescriptorTable(0, text->GetAtlasGPUHandle());
		// マテリアルデータ
		commandList->SetGraphicsRootConstantBufferView(2, material->GetBuffer().GetResource()->GetGPUVirtualAddress());

		// 描画処理
		commandList->DrawIndexedInstanced(text->GetDrawIndexCount(), 1, 0, 0, 0);
	}
}
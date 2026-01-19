#include "CanvasRenderer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/GPUObject/SceneConstBuffer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/CanvasBufferSystem.h>

//============================================================================
//	CanvasRenderer classMethods
//============================================================================

void CanvasRenderer::Init(ID3D12Device8* device, DxShaderCompiler* shaderCompiler, SRVDescriptor* srvDescriptor) {

	// パイプライン作成
	// スプライト用パイプライン
	uint32_t canvasIndex = static_cast<uint32_t>(CanvasType::Sprite);
	pipelines_[canvasIndex] = std::make_unique<PipelineState>();
	pipelines_[canvasIndex]->Create("Object2D.json", device, srvDescriptor, shaderCompiler);
	// テキスト用パイプライン
	canvasIndex = static_cast<uint32_t>(CanvasType::Text);
	pipelines_[canvasIndex] = std::make_unique<PipelineState>();
	pipelines_[canvasIndex]->Create("MSDFText2D.json", device, srvDescriptor, shaderCompiler);
}

void CanvasRenderer::Rendering(CanvasLayer layer, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand) {

	// 描画情報取得
	const auto& system = ObjectManager::GetInstance()->GetSystem<CanvasBufferSystem>();
	const auto& canvasData = system->GetCanvasData(layer);
	if (canvasData.empty()) {
		return;
	}

	// commandList取得
	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	// ブレンドモード
	currentBlendMode_ = kBlendModeCount;
	currentCanvasType_ = CanvasType::Count;
	for (const auto& canvas : canvasData) {

		SetPipeline(commandList, *canvas);
		// カメラの正射影行列設定
		sceneBuffer->SetOrthoProCommand(commandList, 0);

		// 描画
		canvas->DrawCommand(commandList);
	}
}

void CanvasRenderer::SetPipeline(ID3D12GraphicsCommandList6* commandList, const BaseCanvas& canvas) {

	// 別のブレンドが設定されている、もしくはタイプが変わった場合はパイプラインを変更
	BlendMode blendMode = canvas.GetBlendMode();
	CanvasType canvasType = canvas.GetType();
	if (currentBlendMode_ != blendMode ||
		currentCanvasType_ != canvasType) {

		// パイプライン設定
		uint32_t canvasTypeIndex = static_cast<uint32_t>(canvasType);
		commandList->SetGraphicsRootSignature(pipelines_[canvasTypeIndex]->GetRootSignature());
		commandList->SetPipelineState(pipelines_[canvasTypeIndex]->GetGraphicsPipeline(blendMode));
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 現在の設定を更新
		currentBlendMode_ = blendMode;
		currentCanvasType_ = canvasType;
	}
}
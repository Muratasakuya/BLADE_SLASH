#include "CanvasRenderer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/CanvasBufferSystem.h>

//============================================================================
//	CanvasRenderer classMethods
//============================================================================

void CanvasRenderer::Init(ID3D12Device8* device,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler) {

	// パイプライン作成
	// ポストエフェクト無効
	uint32_t renderModeIndex = static_cast<uint32_t>(RenderMode::IrrelevantPostProcess);
	// スプライト用パイプライン
	uint32_t canvasIndex = static_cast<uint32_t>(CanvasType::Sprite);
	pipelines_[renderModeIndex][canvasIndex] = std::make_unique<PipelineState>();
	pipelines_[renderModeIndex][canvasIndex]->Create("IrrelevantPostProcessObject2D.json", device, srvDescriptor, shaderCompiler);
	// テキスト用パイプライン
	canvasIndex = static_cast<uint32_t>(CanvasType::Text);
	pipelines_[renderModeIndex][canvasIndex] = std::make_unique<PipelineState>();
	pipelines_[renderModeIndex][canvasIndex]->Create("IrrelevantPostProcessMSDFText2D.json", device, srvDescriptor, shaderCompiler);

	// ポストエフェクト有効
	renderModeIndex = static_cast<uint32_t>(RenderMode::ApplyPostProcess);
	// スプライト用パイプライン
	canvasIndex = static_cast<uint32_t>(CanvasType::Sprite);
	pipelines_[renderModeIndex][canvasIndex] = std::make_unique<PipelineState>();
	pipelines_[renderModeIndex][canvasIndex]->Create("ApplyPostProcessObject2D.json", device, srvDescriptor, shaderCompiler);
	// テキスト用パイプライン
	canvasIndex = static_cast<uint32_t>(CanvasType::Text);
	pipelines_[renderModeIndex][canvasIndex] = std::make_unique<PipelineState>();
	pipelines_[renderModeIndex][canvasIndex]->Create("ApplyPostProcessMSDFText2D.json", device, srvDescriptor, shaderCompiler);
}

void CanvasRenderer::ApplyPostProcessRendering(CanvasLayer layer, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand) {

	// 描画情報取得
	const auto& system = ObjectManager::GetInstance()->GetSystem<CanvasBufferSystem>();
	const auto& canvasData = system->GetCanvasData(layer);
	if (canvasData.empty()) {
		return;
	}

	// commandList取得
	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	// ブレンドモード
	BlendMode currentBlendMode = kBlendModeCount;
	CanvasType currentCanvasType = CanvasType::Count;
	for (const auto& buffer : canvasData) {

		// ポストエフェクト無効スプライトはスキップ
		if (!buffer->IsPostProcessEnable()) {
			continue;
		}
		// 別のブレンドが設定されている、もしくはタイプが変わった場合はパイプラインを変更
	}
}

void CanvasRenderer::IrrelevantPostProcessRendering(SceneConstBuffer* sceneBuffer, DxCommand* dxCommand) {

	// 描画情報取得
	const auto& system = ObjectManager::GetInstance()->GetSystem<CanvasBufferSystem>();
	// 両方のレイヤを取得
	const auto& preCanvasData = system->GetCanvasData(CanvasLayer::PreModel);
	const auto& postCanvasData = system->GetCanvasData(CanvasLayer::PostModel);
	if (preCanvasData.empty() && postCanvasData.empty()) {
		return;
	}
}
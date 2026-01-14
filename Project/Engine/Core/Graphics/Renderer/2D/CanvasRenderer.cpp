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
	currentBlendMode_ = kBlendModeCount;
	currentCanvasType_ = CanvasType::Count;
	for (const auto& canvas : canvasData) {

		// ポストエフェクト無効スプライトはスキップ
		if (!canvas->IsPostProcessEnable()) {
			continue;
		}
		SetPipeline(commandList, *canvas, RenderMode::ApplyPostProcess);
		// カメラの正射影行列設定
		sceneBuffer->SetOrthoProCommand(commandList, 0);

		// 描画
		canvas->DrawCommand(commandList);
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

	// commandList取得
	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	//============================================================================================================================================
	//	レイヤー：PreModel
	//============================================================================================================================================

	currentBlendMode_ = kBlendModeCount;
	currentCanvasType_ = CanvasType::Count;
	for (auto& canvas : preCanvasData) {

		// ポストエフェクト有効スプライトはスキップ
		if (canvas->IsPostProcessEnable()) {
			continue;
		}
		SetPipeline(commandList, *canvas, RenderMode::IrrelevantPostProcess);
		// カメラの正射影行列設定
		sceneBuffer->SetOrthoProCommand(commandList, 0);

		// 描画
		canvas->DrawCommand(commandList);
	}

	//============================================================================================================================================
	//	レイヤー：PostModel
	//============================================================================================================================================

	currentBlendMode_ = kBlendModeCount;
	currentCanvasType_ = CanvasType::Count;
	for (auto& canvas : postCanvasData) {

		// ポストエフェクト有効スプライトはスキップ
		if (canvas->IsPostProcessEnable()) {
			continue;
		}
		SetPipeline(commandList, *canvas, RenderMode::IrrelevantPostProcess);
		// カメラの正射影行列設定
		sceneBuffer->SetOrthoProCommand(commandList, 0);

		// 描画
		canvas->DrawCommand(commandList);
	}
}

void CanvasRenderer::SetPipeline(ID3D12GraphicsCommandList6* commandList, const BaseCanvas& canvas, RenderMode renderMode) {

	// 別のブレンドが設定されている、もしくはタイプが変わった場合はパイプラインを変更
	BlendMode blendMode = canvas.GetBlendMode();
	CanvasType canvasType = canvas.GetType();
	if (currentBlendMode_ != blendMode ||
		currentCanvasType_ != canvasType) {

		// パイプライン設定
		uint32_t renderModeIndex = static_cast<uint32_t>(renderMode);
		uint32_t canvasTypeIndex = static_cast<uint32_t>(canvasType);
		commandList->SetGraphicsRootSignature(pipelines_[renderModeIndex][canvasTypeIndex]->GetRootSignature());
		commandList->SetPipelineState(pipelines_[renderModeIndex][canvasTypeIndex]->GetGraphicsPipeline(blendMode));
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 現在の設定を更新
		currentBlendMode_ = blendMode;
		currentCanvasType_ = canvasType;
	}
}
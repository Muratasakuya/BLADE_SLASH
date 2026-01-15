#include "BaseLineRenderer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/SceneView.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	BaseLineRenderer classMethods
//============================================================================

void BaseLineRenderer::Init(const std::string& jsonPath, ID3D12Device8* device,
	ID3D12GraphicsCommandList* commandList, SRVDescriptor* srvDescriptor,
	DxShaderCompiler* shaderCompiler, SceneView* sceneView) {

	// Releaseでは作成、処理を行わない
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	commandList_ = nullptr;
	commandList_ = commandList;

	sceneView_ = nullptr;
	sceneView_ = sceneView;

	// 各描画情報を初期化
	renderData_[LineType::None].Init(jsonPath, device, srvDescriptor, shaderCompiler);
	renderData_[LineType::DepthIgnore].Init(EnumAdapter<LineType>::ToString(LineType::DepthIgnore) + jsonPath,
		device, srvDescriptor, shaderCompiler);

	viewProjectionBuffer_.CreateBuffer(device);
	debugSceneViewProjectionBuffer_.CreateBuffer(device);
#endif
}

void BaseLineRenderer::RenderStructure::Init(const std::string& pipelineFile, ID3D12Device8* device,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler) {

	pipeline = std::make_unique<PipelineState>();
	pipeline->Create(pipelineFile, device, srvDescriptor, shaderCompiler);

	vertexBuffer.CreateBuffer(device, kMaxLineCount_ * kVertexCountLine_);
}

void BaseLineRenderer::Execute(bool debugEnable, LineType type) {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	auto& renderData = renderData_[type];
	if (renderData.lineVertices.empty()) {
		return;
	}

	// パイプライン設定
	commandList_->SetGraphicsRootSignature(renderData.pipeline->GetRootSignature());
	commandList_->SetPipelineState(renderData.pipeline->GetGraphicsPipeline());
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	// 頂点バッファ設定
	renderData.vertexBuffer.TransferData(renderData.lineVertices);
	commandList_->IASetVertexBuffers(0, 1, &renderData.vertexBuffer.GetVertexBufferView());

	// 定数バッファ設定
	SetViewProjectionBuffer(debugEnable);

	// 描画実行
	commandList_->DrawInstanced(static_cast<UINT>(renderData.lineVertices.size()), 1, 0, 0);
#endif
}

void BaseLineRenderer::ResetLine() {

	// ライン情報をクリア
	for (auto& [type, renderData] : renderData_) {

		renderData.lineVertices.clear();
	}
}

void BaseLineRenderer::SetViewProjectionBuffer(bool debugEnable) {

	// 次元ごとにカメラ行列を設定
	if (GetLineDimension() == LineDimension::Line2D) {

		viewProjectionBuffer_.TransferData(sceneView_->GetCamera2D()->GetViewProjectionMatrix());
		commandList_->SetGraphicsRootConstantBufferView(0, viewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());
	} else if (GetLineDimension() == LineDimension::Line3D) {
		if (!debugEnable) {

			viewProjectionBuffer_.TransferData(sceneView_->GetCamera()->GetViewProjectionMatrix());
			commandList_->SetGraphicsRootConstantBufferView(0, viewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());
		} else {

			debugSceneViewProjectionBuffer_.TransferData(sceneView_->GetSceneCamera()->GetViewProjectionMatrix());
			commandList_->SetGraphicsRootConstantBufferView(0, debugSceneViewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());
		}
	}
}
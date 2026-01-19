#include "SceneColorGradingPass.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/PostProcess/PostProcessConfig.h>
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Config.h>

//============================================================================
//	SceneColorGradingPass classMethods
//============================================================================

void SceneColorGradingPass::Init(ID3D12Device8* device,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler) {

	// 使用するパイプラインを生成
	pipeline_ = std::make_unique<PipelineState>();
	pipeline_->Create("SceneColorGrading.json", device, srvDescriptor, shaderCompiler);

	// プロセッサを生成
	for (auto& passData : passData_) {

		passData.processor = std::make_unique<ComputePostProcessor>();
		passData.processor->Init(device, srvDescriptor, Config::kWindowWidth, Config::kWindowHeight);
		passData.isExecuted = false;
	}

	// バッファを生成
	buffer_ = std::make_unique<PostProcessBuffer<SceneColorGradingForGPU>>();
	buffer_->Init(device, 2);
}

const D3D12_GPU_DESCRIPTOR_HANDLE& SceneColorGradingPass::GetOutputSRVGPUHandle(bool debugEnable) const {

	return passData_[static_cast<size_t>(debugEnable)].processor->GetSRVGPUHandle();
}

void SceneColorGradingPass::Execute(bool debugEnable, DxCommand* dxCommand, const D3D12_GPU_DESCRIPTOR_HANDLE& inputSRVGPUHandle) {

	// バッファ更新
	buffer_->Update();

	// パスデータ取得
	PassData& passData = passData_[static_cast<size_t>(debugEnable)];

	// スレッドグループ数計算
	UINT threadGroupCountX = DxUtils::RoundUp(static_cast<UINT>(passData.processor->GetTextureSize().x), THREAD_POSTPROCESS_GROUP);
	UINT threadGroupCountY = DxUtils::RoundUp(static_cast<UINT>(passData.processor->GetTextureSize().y), THREAD_POSTPROCESS_GROUP);

	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	// パイプライン設定
	commandList->SetComputeRootSignature(pipeline_->GetRootSignature());
	commandList->SetPipelineState(pipeline_->GetComputePipeline());

	// テクスチャ設定
	commandList->SetComputeRootDescriptorTable(0, passData.processor->GetUAVGPUHandle());
	commandList->SetComputeRootDescriptorTable(1, inputSRVGPUHandle);

	// バッファ設定
	commandList->SetComputeRootConstantBufferView(buffer_->GetRootIndex(), buffer_->GetResource()->GetGPUVirtualAddress());

	// 実行
	commandList->Dispatch(threadGroupCountX, threadGroupCountY, 1);

	// UnorderedAccess -> PixelShader
	dxCommand->TransitionBarriers({ passData.processor->GetOutputTextureResource() },
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	passData.isExecuted = true;
}

void SceneColorGradingPass::ToWrite(DxCommand* dxCommand) {

	for (auto& passData : passData_) {

		// 実行されていなければスキップ
		if (!passData.isExecuted) {
			continue;
		}

		// PixelShader -> UnorderedAccess
		dxCommand->TransitionBarriers({ passData.processor->GetOutputTextureResource() },
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		// フラグをリセット
		passData.isExecuted = false;
	}
}

void SceneColorGradingPass::ImGui() {

	ImGui::SeparatorText("Color Grading Pass");

	if (bufferData_.ImGui()) {

		// バッファ更新
		buffer_->SetParameter(&bufferData_, sizeof(SceneColorGradingForGPU));
	}
}

bool SceneColorGradingForGPU::ImGui() {

	bool changed = false;

	// 露出度
	changed |= ImGui::DragFloat("Exposure", &exposure, 0.001f);
	return changed;
}
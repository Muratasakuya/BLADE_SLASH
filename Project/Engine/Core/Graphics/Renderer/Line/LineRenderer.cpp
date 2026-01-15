#include "LineRenderer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Scene/SceneView.h>

//============================================================================
//	LineRenderer classMethods
//============================================================================

LineRenderer* LineRenderer::instance_ = nullptr;

LineRenderer* LineRenderer::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new LineRenderer();
	}
	return instance_;
}

void LineRenderer::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void LineRenderer::Init(ID3D12Device8* device, ID3D12GraphicsCommandList* commandList,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler, SceneView* sceneView) {

	// 描画機能初期化
	// 2Dライン描画
	renderer2D_ = std::make_unique<LineRenderer2D>();
	renderer2D_->Init("PrimitiveLine.json", device, commandList, srvDescriptor, shaderCompiler, sceneView);
	// 3Dライン描画
	renderer3D_ = std::make_unique<LineRenderer3D>();
	renderer3D_->Init("PrimitiveLine.json", device, commandList, srvDescriptor, shaderCompiler, sceneView);
}

void LineRenderer::DrawDebug() {

	renderer2D_->DrawDebug();
	renderer3D_->DrawDebug();
}

void LineRenderer::ResetLine() {

	renderer2D_->ResetLine();
	renderer3D_->ResetLine();
}

void LineRenderer::ImGui() {

	ImGui::SeparatorText("Renderer 2D");
	{
		ImGui::PushID("LineRenderer2D");

		renderer2D_->ImGui();

		ImGui::PopID();
	}
	ImGui::SeparatorText("Renderer 3D");
	{
		ImGui::PushID("LineRenderer3D");

		renderer3D_->ImGui();

		ImGui::PopID();
	}
}
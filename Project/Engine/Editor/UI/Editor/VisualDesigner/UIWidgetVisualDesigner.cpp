#include "UIWidgetVisualDesigner.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

//============================================================================
//	UIWidgetVisualDesigner classMethods
//============================================================================

void UIWidgetVisualDesigner::Init(const D3D12_GPU_DESCRIPTOR_HANDLE& renderTextureGPUHandle) {

	// GPUハンドル保存
	renderTextureGPUHandle_ = renderTextureGPUHandle;
}

void UIWidgetVisualDesigner::ImGui() {

	ImGui::Begin("Visual Designer", nullptr, ImGuiConfigFlags_ViewportsEnable);

	ImGui::DragFloat2("gameViewSize", &gameViewSize_.x, 1.0f);

	ImGui::Image(ImTextureID(renderTextureGPUHandle_.ptr), gameViewSize_);

	ImGui::End();
}
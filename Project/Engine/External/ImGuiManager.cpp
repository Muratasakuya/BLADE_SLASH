#include "ImGuiManager.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/Config.h>

// imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

//============================================================================
//	ImGuiManager classMethods
//============================================================================

void ImGuiManager::Init(HWND hwnd, UINT bufferCount, ID3D12Device* device, ID3D12CommandQueue* commandQueue, SRVDescriptor* srvDescriptor) {

	// マルチビューポート設定
	enableMultiViewport_ = true;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// コンフィグ設定
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 
	if (enableMultiViewport_) {

		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	}

	ImGui::StyleColorsDark(); 
	//Win32初期化
	ImGui_ImplWin32_Init(hwnd);
	
	// DX12初期化 
	ImGui_ImplDX12_InitInfo dxInitInfo = {};
	dxInitInfo.Device = device;
	dxInitInfo.CommandQueue = commandQueue;
	dxInitInfo.NumFramesInFlight = bufferCount;
	dxInitInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxInitInfo.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dxInitInfo.SrvDescriptorHeap = srvDescriptor->GetDescriptorHeap();
	dxInitInfo.LegacySingleSrvCpuDescriptor = srvDescriptor->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	dxInitInfo.LegacySingleSrvGpuDescriptor = srvDescriptor->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	ImGui_ImplDX12_Init(&dxInitInfo);

	// SRVを進める
	srvDescriptor->IncrementIndex();

	//========================================================================
	//	imguiConfig
	//========================================================================

	// ImGuiのフォント設定
	ImFontConfig cfg{};
	cfg.FontNo = 0;
	const char* fontPath = "C:\\Windows\\Fonts\\meiryob.ttc";
	io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath, 24.0f, &cfg, io.Fonts->GetGlyphRangesJapanese());

	// 背景色設定
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Text
	colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);

	// Window
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.09f, 0.10f, 0.98f);

	// Borders
	colors[ImGuiCol_Border] = ImVec4(0.26f, 0.27f, 0.29f, 0.60f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Frame (Input/Slider/Combo)
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);

	// Titlebar
	colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.10f, 0.11f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.08f, 0.09f, 1.00f);

	// Menu bar
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.13f, 0.14f, 1.00f);

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.25f, 0.27f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.32f, 0.35f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.34f, 0.36f, 0.40f, 1.00f);

	// Checkmark / Slider grab
	colors[ImGuiCol_CheckMark] = ImVec4(0.14f, 0.48f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.14f, 0.48f, 0.90f, 0.75f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.14f, 0.48f, 0.90f, 1.00f);

	// Buttons
	colors[ImGuiCol_Button] = ImVec4(0.18f, 0.19f, 0.21f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.14f, 0.48f, 0.90f, 1.00f);

	// Header (TreeNode / Selectable / CollapsingHeader)
	colors[ImGuiCol_Header] = ImVec4(0.18f, 0.19f, 0.21f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.14f, 0.48f, 0.90f, 0.90f);

	// Separator / ResizeGrip
	colors[ImGuiCol_Separator] = ImVec4(0.26f, 0.27f, 0.29f, 0.60f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.48f, 0.90f, 0.60f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.48f, 0.90f, 1.00f);

	colors[ImGuiCol_ResizeGrip] = ImVec4(0.14f, 0.48f, 0.90f, 0.18f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.14f, 0.48f, 0.90f, 0.45f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.14f, 0.48f, 0.90f, 0.75f);

	// Tabs（Docking用に重要）
	colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.48f, 0.90f, 0.85f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.14f, 0.48f, 0.90f, 1.00f);
	colors[ImGuiCol_TabDimmed] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
	colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.14f, 0.48f, 0.90f, 0.60f);

	// Docking
	colors[ImGuiCol_DockingPreview] = ImVec4(0.14f, 0.48f, 0.90f, 0.35f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);

	// Plots
	colors[ImGuiCol_PlotLines] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.14f, 0.48f, 0.90f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.14f, 0.48f, 0.90f, 1.00f);

	// Selection / DragDrop
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.14f, 0.48f, 0.90f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.14f, 0.48f, 0.90f, 0.95f);

	// Nav highlight
	colors[ImGuiCol_NavHighlight] = ImVec4(0.14f, 0.48f, 0.90f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.14f, 0.48f, 0.90f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.45f);

	// 角の丸み
	style.FrameRounding = 2;

	// ドックウィンドウ間のサイズ
	style.DockingSeparatorSize = 2.0f;
}

void ImGuiManager::Begin() {

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End() {

	ImGui::Render();
}

void ImGuiManager::Draw(ID3D12GraphicsCommandList* commandList) {

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

	// マルチビューポート有効の時のみ
	if (enableMultiViewport_) {

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {

			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
}

void ImGuiManager::Finalize() {

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
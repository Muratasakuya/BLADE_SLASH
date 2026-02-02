#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>

// directX
#include <d3d12.h>
// imgui
#include <imgui.h>

namespace SakuEngine {

	//============================================================================
	//	UIVisualDesignerPanel class
	//	UIの現在の見た目を表示、デザインするパネル
	//============================================================================
	class UIVisualDesignerPanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIVisualDesignerPanel() = default;
		~UIVisualDesignerPanel() = default;

		// エディター
		void ImGui(UIToolContext& context) override;

		//--------- accessor -----------------------------------------------------

		// GPUハンドル設定
		void SetTextureGPUHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& handle) { textureGPUHandle_ = handle; }

		// エディタの名前、パネルに表示する
		const char* GetName() const override { return "Visual Designer"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 描画用テクスチャのGPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE textureGPUHandle_{};

		// エディタデバッグ
		ImVec2 gameViewSize_ = ImVec2(896.0f, 504.0f);

		//--------- functions ----------------------------------------------------

	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Panels/Interface/UIWidgetEditorPanel.h>
#include <Engine/MathLib/Vector2.h>

// directX
#include <d3d12.h>
// imgui
#include <imgui.h>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetVisualDesigner class
	//	UIウィジェットの現在の見た目を確認、編集するためのクラス
	//============================================================================
	class UIWidgetVisualDesigner :
		public IUIWidgetEditorPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetVisualDesigner() = default;
		~UIWidgetVisualDesigner() = default;

		// 初期化
		void Init(const D3D12_GPU_DESCRIPTOR_HANDLE& renderTextureGPUHandle);

		// エディター
		void Draw(UIWidgetEditorContext& context) override;

		//--------- accessor -----------------------------------------------------

		const char* GetPanelName() const override { return "Visual Designer"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 描画用テクスチャのGPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE renderTextureGPUHandle_{};

		// エディタデバッグ
		ImVec2 gameViewSize_ = ImVec2(1024.0f, 576.0f);

		// 操作用
		bool isDragging_ = false;
		Vector2 dragStartMouse_{};
		Vector2 dragStartOffsetsLT_{};
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Object/Data/Canvas/CanvasCommon.h>

// c++
#include <memory>

namespace SakuEngine {

	// front
	class DxCommand;
	class SceneConstBuffer;
	class SRVDescriptor;
	class DxShaderCompiler;

	//============================================================================
	//	CanvasRenderer class
	//	2D描画を行うクラス
	//============================================================================
	class CanvasRenderer {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		CanvasRenderer() = default;
		~CanvasRenderer() = default;

		// 描画用パイプラインを作成し初期化
		void Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

		// 指定レイヤ別に描画
		// ポストエフェクト有効
		void ApplyPostProcessRendering(CanvasLayer layer, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
		// ポストエフェクト無効
		void IrrelevantPostProcessRendering(SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);

		//--------- accessor -----------------------------------------------------

	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// 描画モード
		enum class RenderMode {

			IrrelevantPostProcess, // ポストエフェクト無効
			ApplyPostProcess,      // ポストエフェクト有効
			Count
		};

		//--------- variables ----------------------------------------------------

		static constexpr uint32_t kRenderModeCount_ = static_cast<uint32_t>(RenderMode::Count);
		static constexpr uint32_t kCanvasCount_ = static_cast<uint32_t>(CanvasType::Count);

		// 描画用パイプライン
		std::array<std::array<std::unique_ptr<PipelineState>, kCanvasCount_>, kRenderModeCount_> pipelines_;

		//--------- functions ----------------------------------------------------

	};
} // SakuEngine
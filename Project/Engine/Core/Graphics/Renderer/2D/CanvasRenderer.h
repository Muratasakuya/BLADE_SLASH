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
		void Init(ID3D12Device8* device, DxShaderCompiler* shaderCompiler, SRVDescriptor* srvDescriptor);

		// 指定レイヤ別に描画
		void Rendering(CanvasLayer layer, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		static constexpr uint32_t kCanvasCount_ = static_cast<uint32_t>(CanvasType::Count);

		// 描画用パイプライン
		std::array<std::unique_ptr<PipelineState>, kCanvasCount_> pipelines_;

		// 比較用列挙
		BlendMode currentBlendMode_ = kBlendModeCount;
		CanvasType currentCanvasType_ = CanvasType::Count;

		//--------- functions ----------------------------------------------------

		// パイプラインのセット
		void SetPipeline(ID3D12GraphicsCommandList6* commandList, const BaseCanvas& canvas);
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Core/Graphics/PostProcess/Core/ComputePostProcessor.h>
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBuffer.h>

namespace SakuEngine {

	// front
	class SRVDescriptor;
	class DxShaderCompiler;
	class DxCommand;

	//============================================================================
	//	SceneColorGradingBuffer structure
	//	シーンの色を調整するポストプロセスに必要なバッファ
	//============================================================================
	struct SceneColorGradingForGPU {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		SceneColorGradingForGPU() = default;
		~SceneColorGradingForGPU() = default;

		// エディター
		bool ImGui();

		//--------- properties ---------------------------------------------------

		// 露出度
		float exposure = 1.5f;

		float pad[3];
	};

	//============================================================================
	//	SceneColorGradingPass class
	//	シーンの色を調整するポストプロセスパス
	//============================================================================
	class SceneColorGradingPass :
		public IGameEditor {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		SceneColorGradingPass() :IGameEditor("ColorGradingPass") {}
		~SceneColorGradingPass() = default;

		// 初期化
		void Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

		// 実行
		void Execute(bool debugEnable, DxCommand* dxCommand, const D3D12_GPU_DESCRIPTOR_HANDLE& inputSRVGPUHandle);

		// 次フレームに備え、出力テクスチャをUAV書き込み状態へ戻す
		void ToWrite(DxCommand* dxCommand);

		// エディター
		void ImGui() override;

		//--------- accessor -----------------------------------------------------

		// 色調整後の出力SRVハンドルを取得
		const D3D12_GPU_DESCRIPTOR_HANDLE& GetOutputSRVGPUHandle(bool debugEnable) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// シーンごとに持つ
		struct PassData {

			// ComputeShader実行用プロセッサ
			std::unique_ptr<ComputePostProcessor> processor;

			// 実行済みフラグ
			bool isExecuted = false;
		};

		//--------- variables ----------------------------------------------------

		// パイプライン
		std::unique_ptr<PipelineState> pipeline_;
		// シーンの数、サイズに依存するのでパス毎に持つ
		static const uint32_t kSceneCount_ = 2;
		std::array<PassData, kSceneCount_> passData_;
		// バッファ
		std::unique_ptr<PostProcessBuffer<SceneColorGradingForGPU>> buffer_;
		SceneColorGradingForGPU bufferData_;

		//--------- functions ----------------------------------------------------

	};
}
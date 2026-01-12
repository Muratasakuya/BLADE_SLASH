#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>

// c++
#include <memory>

namespace SakuEngine {

	// front
	class DxCommand;
	class SceneConstBuffer;
	class SRVDescriptor;
	class DxShaderCompiler;

	//============================================================================
	//	TextRenderer class
	//	テキスト描画管理クラス
	//============================================================================
	class TextRenderer {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		TextRenderer() = default;
		~TextRenderer() = default;

		// テキスト用パイプラインを作成し初期化
		void Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

		// テキスト描画
		void Rendering(SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);

		//--------- accessor -----------------------------------------------------

	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		std::unique_ptr<PipelineState> pipeline_;

		//--------- functions ----------------------------------------------------

	};
} // SakuEngine
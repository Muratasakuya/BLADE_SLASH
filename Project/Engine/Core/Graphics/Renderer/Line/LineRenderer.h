#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/GPUObject/VertexBuffer.h>
#include <Engine/Collision/CollisionGeometry.h>
#include <Engine/MathLib/MathUtils.h>
#include <Engine/Core/Graphics/Renderer/Line/Dimensions/LineRenderer2D.h>
#include <Engine/Core/Graphics/Renderer/Line/Dimensions/LineRenderer3D.h>

// c++
#include <memory>

namespace SakuEngine {

	//============================================================================
	//	LineRenderer class
	//	デバッグ可視化用のライン描画を担当。各種プリミティブ線の生成と描画実行を行う。
	//============================================================================
	class LineRenderer {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		// 必要なパイプライン/バッファを初期化し、参照オブジェクトを記録
		void Init(ID3D12Device8* device, ID3D12GraphicsCommandList* commandList,
			SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler,
			SceneView* sceneView);

		// 2点間のラインを追加登録する
		template <typename T>
		void DrawLine(const T& pointA, const T& pointB, const Color& color, LineType type = LineType::None);

		// 蓄積されたラインをGPU転送して描画する
		void ExecuteLine(bool debugEnable, LineType type);
		// 追加済みの全ラインをクリアする
		void ResetLine();

		//--------- accessor -----------------------------------------------------

		// 描画機能取得
		LineRenderer2D* Get2D() const { return renderer2D_.get(); }
		LineRenderer3D* Get3D() const { return renderer3D_.get(); }

		// singleton取得/破棄
		static LineRenderer* GetInstance();
		static void Finalize();
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		static LineRenderer* instance_;

		// ライン描画クラス
		std::unique_ptr<LineRenderer2D> renderer2D_;
		std::unique_ptr<LineRenderer3D> renderer3D_;

		//--------- functions ----------------------------------------------------

		LineRenderer() = default;
		~LineRenderer() = default;
		LineRenderer(const LineRenderer&) = delete;
		LineRenderer& operator=(const LineRenderer&) = delete;
	};

	//============================================================================
	//	LineRenderer templateMethods
	//============================================================================

	template<typename T>
	inline void LineRenderer::DrawLine(const T& pointA, const T& pointB, const Color& color, LineType type) {

		if constexpr (std::is_same_v<T, Vector2>) {

			renderer2D_->DrawLine(pointA, pointB, color, type);
		} else if constexpr (std::is_same_v<T, Vector3>) {

			renderer3D_->DrawLine(pointA, pointB, color, type);
		} else {

			static_assert(false, "Unsupported type for DrawLine");
		}
	}
}; // SakuEngine
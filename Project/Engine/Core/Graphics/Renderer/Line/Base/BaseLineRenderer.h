#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/GPUObject/VertexBuffer.h>
#include <Engine/MathLib/MathUtils.h>

namespace SakuEngine {

	// front
	class SceneView;
	class SRVDescriptor;
	class DxShaderCompiler;

	//============================================================================
	//	enum class
	//============================================================================

	// 線の種類
	enum class LineType {

		None,        // 通常描画
		DepthIgnore, // 深度値無視
	};

	// 描画次元
	enum class LineDimension {

		Line2D,
		Line3D,
		Count
	};

	//============================================================================
	//	BaseLineRenderer class
	//	ライン描画の基底クラス
	//============================================================================
	class BaseLineRenderer {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		BaseLineRenderer() = default;
		virtual ~BaseLineRenderer() = default;

		// 初期化
		void Init(const std::string& jsonPath, ID3D12Device8* device, ID3D12GraphicsCommandList* commandList,
			SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler, SceneView* sceneView);

		// ライン設定を保存
		template <typename T>
		void DrawLine(const T& pointA, const T& pointB, const Color& color, LineType type = LineType::None);

		// ライン描画実行
		void Execute(bool debugEnable, LineType type);
		// ライン情報リセット
		void ResetLine();

		// エディター
		virtual void ImGui() {}
		virtual void DrawDebug() {}

		//--------- accessor -----------------------------------------------------

		// 描画次元を取得
		virtual LineDimension GetLineDimension() const = 0;
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// 1頂点の線描画データ(位置/色)
		struct LineVertex {

			Vector4 pos;
			Color color;
		};
		// ライン描画に必要なパイプラインと頂点バッファ/頂点配列
		struct RenderStructure {

			std::unique_ptr<PipelineState> pipeline;

			std::vector<LineVertex> lineVertices;
			VertexBuffer<LineVertex> vertexBuffer;

			void Init(const std::string& pipelineFile, ID3D12Device8* device,
				SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);
		};

		//--------- variables ----------------------------------------------------

		// 線分の最大数
		static constexpr const uint32_t kMaxLineCount_ = 8196;
		// 線分の頂点数
		static constexpr const uint32_t kVertexCountLine_ = 2;

		ID3D12GraphicsCommandList* commandList_;
		SceneView* sceneView_;

		// カメラ視点
		DxConstBuffer<Matrix4x4> viewProjectionBuffer_;
		DxConstBuffer<Matrix4x4> debugSceneViewProjectionBuffer_;

		// 描画情報
		std::unordered_map<LineType, RenderStructure> renderData_;

		//--------- functions ----------------------------------------------------

		// カメラ視点行列設定
		void SetViewProjectionBuffer(bool debugEnable);
	};

	//============================================================================
	//	BaseLineRenderer templateMethods
	//============================================================================

	template<typename T>
	inline void BaseLineRenderer::DrawLine(const T& pointA, const T& pointB, const Color& color, LineType type) {

		auto& lineVertices = renderData_[type].lineVertices;
		ASSERT(lineVertices.size() < kMaxLineCount_ * kVertexCountLine_, "exceeded the upper limit line");

		Vector4 pointA4{};
		Vector4 pointB4{};
		if constexpr (std::is_same_v<T, Vector2>) {

			pointA4 = Vector4(pointA.x, pointA.y, 0.0f, 1.0f);
			pointB4 = Vector4(pointB.x, pointB.y, 0.0f, 1.0f);
		} else if constexpr (std::is_same_v<T, Vector3>) {

			pointA4 = Vector4(pointA.x, pointA.y, pointA.z, 1.0f);
			pointB4 = Vector4(pointB.x, pointB.y, pointB.z, 1.0f);
		}
		// 頂点追加
		lineVertices.emplace_back(pointA4, color);
		lineVertices.emplace_back(pointB4, color);
	}
}
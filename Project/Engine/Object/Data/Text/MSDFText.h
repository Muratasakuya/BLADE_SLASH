#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetStructure.h>
#include <Engine/Core/Graphics/GPUObject/VertexBuffer.h>
#include <Engine/Core/Graphics/GPUObject/IndexBuffer.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Object/Data/Text/MSDFFont.h>
#include <Engine/Object/Data/Material/Material.h>
#include <Engine/Object/Data/Transform/Transform.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Engine/Object/Data/Canvas/CanvasCommon.h>

// directX
#include <DirectXMath.h>

namespace SakuEngine {

	// テキストの測定結果
	struct MSDFTextBounds {

		// レイアウト結果のAABB
		Vector2 min{};
		Vector2 max{};

		// 幅、高さ取得
		float Width() const { return max.x - min.x; }
		float Height() const { return max.y - min.y; }
	};

	//============================================================================
	//	MSDFText class
	//	MSDFテキスト描画データ
	//============================================================================
	class MSDFText :
		public BaseCanvas {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		MSDFText() = default;
		// 初期化
		MSDFText(ID3D12Device* device, Asset* asset, const MSDFFont* font, uint32_t maxGlyphs = 256);
		~MSDFText() = default;

		// エディター
		void ImGui(float itemSize);

		// 更新
		void UpdateVertex(const TextTransform2D& transform);

		// 描画リソースの設定
		void SetRenderResources(uint32_t objectId);
		// 描画コマンド
		void DrawCommand(ID3D12GraphicsCommandList6* commandList) override;

		//--------- accessor -----------------------------------------------------

		// 文字列設定
		void SetText(const std::string& utf8);
		// 整数値を文字列化して設定
		void SetText(int32_t value);
		// 浮動小数点、小数点以下の桁数を指定
		void SetText(float value, int32_t precision);

		// フォントサイズの設定
		void SetFontSizePx(float pixel);

		// フォントサイズ取得
		float GetFontSize() const { return fontSize_; }
		// 文字列取得
		const std::string& GetText() const { return textUtf8_; }
		const std::vector<char32_t>& GetCodepoints() const { return codepoints_; }
		// 文字数取得
		uint32_t GetGlyphCount() const { return static_cast<uint32_t>(codepoints_.size()); }

		// 描画されたグリフ数
		uint32_t GetRenderedGlyphCount() const { return renderedGlyphCount_; }
		// 指定文字のピボット取得
		Vector2 GetGlyphPivot(uint32_t i) const { return glyphPivots_[i]; }

		// 描画情報取得
		uint32_t GetDrawIndexCount() const { return drawIndexCount_; }
		const VertexBuffer<MSDFTextVertexData>& GetVertexBuffer() const { return vertexBuffer_; }
		const IndexBuffer& GetIndexBuffer() const { return indexBuffer_; }

		// アトラステクスチャのGPUハンドル取得
		const D3D12_GPU_DESCRIPTOR_HANDLE& GetAtlasGPUHandle() const { return font_->GetAtlasGPUHandle(); }
		const MSDFTextBounds& GetBounds() const { return bounds_; }

		// フォント取得
		const MSDFFont& GetFont() const { return *font_; }

		// 描画タイプ取得
		CanvasType GetType() const override { return CanvasType::Text; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		ID3D12Device* device_ = nullptr;
		Asset* asset_ = nullptr;
		const MSDFFont* font_ = nullptr;

		// 表示する文字列
		std::string textUtf8_;
		std::vector<char32_t> codepoints_;

		// 最大文字数
		uint32_t maxGlyphs_ = 0;
		// フォントサイズ
		float fontSize_ = 32.0f;
		// 文字間隔
		float charSpacing_ = 0.0f;

		MSDFTextBounds bounds_{};
		// 描画される文字数
		uint32_t renderedGlyphCount_ = 0;
		std::vector<Vector2> glyphPivots_;

		// buffer
		uint32_t drawIndexCount_ = 0;
		VertexBuffer<MSDFTextVertexData> vertexBuffer_;
		IndexBuffer indexBuffer_;

		// 変更があったか
		bool dirtyMesh_ = true;
		// トランスフォームを前回から保持
		TextTransform2D prevTransform_{};

		// エディター
		InputImGui inputText_;

		//--------- functions ----------------------------------------------------

		// 必要に応じて頂点バッファの容量を拡張
		void EnsureCapacity(uint32_t glyphCount);
		void BuildIndexBuffer();
		void RebuildMeshCPU(const TextTransform2D& transform);
		bool IsDirtyTransform(const TextTransform2D& transform) const;
	};
} // namespace SakuEngine
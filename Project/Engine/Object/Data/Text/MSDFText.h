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
#include <Engine/Object/Data/Canvas/CanvasCommon.h>
#include <Engine/Object/Data/Text/Builder/MSDFTextMeshBuilder.h>
#include <Engine/Object/Data/Text/Generator/Interface/ITextGenerator.h>

namespace SakuEngine {

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
		MSDFText(ID3D12Device* device, Asset* asset, const MSDFFont* font, uint32_t maxGlyphs);
		~MSDFText() = default;

		// コピー禁止
		MSDFText(const MSDFText&) = delete;
		MSDFText& operator=(const MSDFText&) = delete;
		// ムーブ許可
		MSDFText(MSDFText&&) noexcept = default;
		MSDFText& operator=(MSDFText&&) noexcept = default;

		// 頂点更新
		void UpdateVertex(const TextTransform2D& transform);

		// 描画用リソース設定
		void SetRenderResources(uint32_t objectId);
		// 描画コマンド
		void DrawCommand(ID3D12GraphicsCommandList6* commandList) override;

		// エディター
		void ImGui(float itemSize);

		//--------- accessor -----------------------------------------------------

		// 文字列設定
		void SetText(const std::string& utf8);
		void SetText(int32_t value);
		void SetText(float value, int32_t precision = 2);

		// 文字列取得
		const std::string& GetText() const { return textUtf8_; }
		const std::vector<char32_t>& GetCodepoints() const { return codepoints_; }
		// 文字数
		uint32_t GetGlyphCount() const { return static_cast<uint32_t>(codepoints_.size()); }
		// 描画される文字数
		uint32_t GetRenderedGlyphCount() const { return renderedGlyphCount_; }

		// タイプ取得
		CanvasType GetType() const override { return CanvasType::Text; }

		// フォント取得
		const MSDFFont& GetFont() const { return *font_; }
		// フォントサイズの取得
		float GetFontSize() const { return fontSize_; }
		// グリフのピボット取得
		Vector2 GetGlyphPivot(uint32_t i) const { return glyphPivots_[i]; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		ID3D12Device* device_ = nullptr;
		Asset* asset_ = nullptr;
		const MSDFFont* font_ = nullptr;

		// 表示文字列
		std::string textUtf8_ = "";
		std::vector<char32_t> codepoints_;
		// 最大文字数
		uint32_t maxGlyphs_ = 0;
		// 現在使用しているアトラステクスチャ
		std::string atlasTextureName_;

		// 表示設定
		float fontSize_ = 32.0f;
		float charSpacing_ = 0.0f;
		TextSpacingParams spacing_{};
		// テキスト矩形情報
		MSDFTextBounds bounds_{};
		uint32_t renderedGlyphCount_ = 0;
		std::vector<Vector2> glyphPivots_;

		// 描画バッファ
		uint32_t drawIndexCount_ = 0;
		VertexBuffer<MSDFTextVertexData> vertexBuffer_;
		IndexBuffer indexBuffer_;

		// 比較用トランスフォーム
		bool dirtyMesh_ = true;
		TextTransform2D prevTransform_{};

		// メッシュ構築
		MSDFTextMeshBuilder meshBuilder_{};
		// テキスト構築
		std::unique_ptr<ITextGenerator> textGenerator_;

		// エディター
		InputImGui inputText_{};
		// floatのプレビュー用
		float previewFloat_ = 0.0f;
		int32_t previewPrecision_ = 2;
		// テキストジェネレータの選択
		TextGeneratorType selectGeneratorType_ = TextGeneratorType::Clock;

		//--------- functions ----------------------------------------------------

		// 初期化、作成
		void Create();

		// バッファ確保
		void EnsureCapacity(uint32_t glyphCount);
		// インデックスバッファ構築
		void BuildIndexBuffer();

		// 変換行列変更検出
		bool IsDirtyTransform(const TextTransform2D& transform) const;
		// テキスト更新
		void UpdateTextFromGenerator();
	};
} // namespace SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Text/Builder/MSDFTextTypes.h>
#include <Engine/Object/Data/Text/MSDFFont.h>
#include <Engine/Object/Data/Transform/Transform.h>

namespace SakuEngine {

	//============================================================================
	//	MSDFTextMeshBuilder class
	//	テキストメッシュ構築クラス
	//============================================================================
	class MSDFTextMeshBuilder {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// ビルド用入力パラメータ
		struct BuildInput {

			// フォントと文字コード
			const MSDFFont* font = nullptr;
			const std::vector<char32_t>* codepoints = nullptr;
			// 最大文字数
			uint32_t maxGlyphs = 256;

			// フォントサイズと文字間隔
			float fontSizePx = 32.0f;
			float charSpacingPx = 0.0f;

			// 文字間隔パラメータ
			TextSpacingParams spacing{};
		};
		// ビルド結果
		struct BuildOutput {

			// 描画に使うデータ数
			uint32_t renderedGlyphCount = 0;
			uint32_t drawIndexCount = 0;

			// バウンディング
			MSDFTextBounds bounds{};
			std::vector<Vector2> glyphPivots;
		};
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		MSDFTextMeshBuilder() = default;
		~MSDFTextMeshBuilder() = default;

		// メッシュ構築
		bool Build(const BuildInput& in, const TextTransform2D& transform,
			std::vector<MSDFTextVertexData>& outVertices, BuildOutput& out) const;

		//--------- accessor -----------------------------------------------------

	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// グリフ配置情報
		struct GlyphPlacement {

			float advance = 0.0f; // ペン進行量
			float offset = 0.0f;  // 描画Xオフセット
			bool useFixed = false;
		};

		//--------- functions ----------------------------------------------------

		// 自動固定幅算出
		static float ComputeAutoFixedAdvance(const MSDFFont& font, const TextSpacingParams& spacing,
			const std::vector<char32_t>& codepoints, float scale);
		// 固定幅算出
		static float ResolveFixedAdvance(const MSDFFont& font, const TextSpacingParams& spacing,
			const std::vector<char32_t>& codepoints, float scale);
		// 固定幅を使うか判定
		static bool ShouldUseFixedAdvance(char32_t codepoint, const TextSpacingParams& spacing);
		// グリフ配置計算
		static GlyphPlacement ComputeGlyphPlacement(const MSDFGlyph& glyph,
			char32_t codepoint, float scale, float fixedAdvancePx, const TextSpacingParams& spacing);
	};
} // SakuEngine
#include "MSDFTextMeshBuilder.h"

using namespace SakuEngine;

//============================================================================
//	MSDFTextMeshBuilder classMethods
//============================================================================

namespace {

	// 2D四角形
	struct Quad2D {
		float x0, y0; // min(left, top)
		float x1, y1; // max(right, bottom)
	};
	struct UVRect {

		float u0, v0; // min
		float u1, v1; // max
	};

	// 最小/最大正規化
	void NormalizeMinMax(float& a, float& b) {
		if (b < a) {
			(std::swap)(a, b);
		}
	}

	// 0-1クランプ
	float Clamp01(float v) {
		return (std::min)(1.0f, (std::max)(0.0f, v));
	}

	// グリフの四角形作成
	Quad2D MakeGlyphQuadYDown(float penX, float penY, const MSDFPlaneBounds& pb, float scale) {

		Quad2D quad{};
		quad.x0 = penX + pb.left * scale;
		quad.x1 = penX + pb.right * scale;
		quad.y0 = penY + pb.top * scale;
		quad.y1 = penY + pb.bottom * scale;
		NormalizeMinMax(quad.x0, quad.x1);
		NormalizeMinMax(quad.y0, quad.y1);
		return quad;
	}
	// グリフのUV矩形作成
	UVRect MakeGlyphUV(const MSDFAtlasBounds& ab, float invAtlasW, float invAtlasH) {

		UVRect uv{};
		uv.u0 = ab.left * invAtlasW;
		uv.u1 = ab.right * invAtlasW;
		uv.v0 = ab.top * invAtlasH;
		uv.v1 = ab.bottom * invAtlasH;
		NormalizeMinMax(uv.u0, uv.u1);
		NormalizeMinMax(uv.v0, uv.v1);
		return uv;
	}
	// 四角形頂点書き込み
	void WriteQuadVertices(MSDFTextVertexData* vertices, const Quad2D& q, const UVRect& uv) {

		vertices[0].pos = Vector2(q.x0, q.y0);
		vertices[0].texcoord = Vector2(uv.u0, uv.v0);

		vertices[1].pos = Vector2(q.x0, q.y1);
		vertices[1].texcoord = Vector2(uv.u0, uv.v1);

		vertices[2].pos = Vector2(q.x1, q.y0);
		vertices[2].texcoord = Vector2(uv.u1, uv.v0);

		vertices[3].pos = Vector2(q.x1, q.y1);
		vertices[3].texcoord = Vector2(uv.u1, uv.v1);
	}
	// 四角形の中心取得
	Vector2 QuadCenter(const Quad2D& q) {
		return Vector2((q.x0 + q.x1) * 0.5f, (q.y0 + q.y1) * 0.5f);
	}
	// 四角形頂点オフセットX/Y/XY
	void OffsetQuadVerticesX(MSDFTextVertexData* vertices, float dx) {
		vertices[0].pos.x += dx; vertices[1].pos.x += dx; vertices[2].pos.x += dx; vertices[3].pos.x += dx;
	}
	void OffsetQuadVerticesY(MSDFTextVertexData* vertices, float dy) {
		vertices[0].pos.y += dy; vertices[1].pos.y += dy; vertices[2].pos.y += dy; vertices[3].pos.y += dy;
	}
	void OffsetQuadVerticesXY(MSDFTextVertexData* vertices, float dx, float dy) {
		vertices[0].pos.x += dx; vertices[0].pos.y += dy;
		vertices[1].pos.x += dx; vertices[1].pos.y += dy;
		vertices[2].pos.x += dx; vertices[2].pos.y += dy;
		vertices[3].pos.x += dx; vertices[3].pos.y += dy;
	}

	// ラインボックス上下計算
	void ComputeLineBoxTopBottomYDown(const MSDFMetrics& metrics, float penYBaseline,
		float scale, float& outTop, float& outBottom) {

		outTop = penYBaseline + metrics.ascender * scale;
		outBottom = penYBaseline + metrics.descender * scale;
		NormalizeMinMax(outTop, outBottom);
	}

	// 数値っぽい記号かどうか
	bool IsNumericSet(char32_t c) {

		if (U'0' <= c && c <= U'9') {
			return true;
		}
		if (U'０' <= c && c <= U'９') {
			return true;
		}
		switch (c) {
		case U'+': case U'-': case U'*': case U'/': case U'\\':
		case U'.': case U',': case U':': case U';': case U'%':
		case U'!': case U'?': case U'=': case U'(': case U')':
		case U'[': case U']': case U'{': case U'}': case U'_':
			return true;
		default:
			return false;
		}
	}
} // namespace

float MSDFTextMeshBuilder::ComputeAutoFixedAdvance(const MSDFFont& font,
	const TextSpacingParams& spacing, const std::vector<char32_t>& codepoints, float scale) {

	float maxAdvance = 0.0f;
	// 	最大進行量を探す
	auto consider = [&](char32_t cp) {
		const MSDFGlyph* g = font.FindGlyph(cp);
		maxAdvance = (std::max)(maxAdvance, g->advance);
		};

	// 数字/記号セットのみに適用する場合
	if (spacing.applyOnlyToNumericSet) {

		for (char32_t cp = U'0'; cp <= U'9'; ++cp) {
			consider(cp);
		}
		for (char32_t cp = U'０'; cp <= U'９'; ++cp) {
			consider(cp);
		}

		// 記号類
		static constexpr std::array<char32_t, 14> kSymbols = {
			U'+',U'-',U'*',U'/',U'.',U',',U':',U';',U'%',U'!',U'?',U'=',U'(',U')'
		};
		for (char32_t cp : kSymbols) {
			consider(cp);
		}
	} else {
		// 全コードポイント走査
		for (char32_t cp : codepoints) {
			consider(cp);
		}
	}

	// 保険
	if (maxAdvance <= 0.0f) {
		const MSDFMetrics& m = font.GetMetrics();
		maxAdvance = (m.emSize > 0.0f) ? m.emSize : 1.0f;
	}
	return maxAdvance * scale;
}

float MSDFTextMeshBuilder::ResolveFixedAdvance(const MSDFFont& font,
	const TextSpacingParams& spacing, const std::vector<char32_t>& codepoints, float scale) {

	// 固定幅モードでなければ0
	if (spacing.mode != TextSpacingMode::FixedAdvance) {
		return 0.0f;
	}
	// 明示的に指定されていればそれを使う
	if (0.0f < spacing.fixedAdvance) {
		return spacing.fixedAdvance;
	}
	return ComputeAutoFixedAdvance(font, spacing, codepoints, scale);
}

bool MSDFTextMeshBuilder::ShouldUseFixedAdvance(char32_t codepoint, const TextSpacingParams& spacing) {

	// 固定幅モードでなければ使わない
	if (spacing.mode != TextSpacingMode::FixedAdvance) {
		return false;
	}
	// 数字/記号セットのみに適用する場合
	if (!spacing.applyOnlyToNumericSet) {
		return true;
	}
	return IsNumericSet(codepoint);
}

MSDFTextMeshBuilder::GlyphPlacement MSDFTextMeshBuilder::ComputeGlyphPlacement(const MSDFGlyph& glyph,
	char32_t codepoint, float scale, float fixedAdvancePx, const TextSpacingParams& spacing) {

	// 基本情報
	GlyphPlacement placement{};
	const float tightAdvance = glyph.advance * scale;
	placement.advance = tightAdvance;
	placement.offset = 0.0f;
	placement.useFixed = false;

	// 固定幅を使わない場合は終了
	if (!ShouldUseFixedAdvance(codepoint, spacing)) {
		return placement;
	}
	// 固定幅計算
	const float fixed = (fixedAdvancePx > 0.0f) ? fixedAdvancePx : tightAdvance;
	placement.advance = (std::max)(tightAdvance, fixed);
	// オフセット計算
	const float align = Clamp01(spacing.cellAlign);
	placement.offset = (placement.advance - tightAdvance) * align;
	// フラグセット
	placement.useFixed = true;
	return placement;
}

bool MSDFTextMeshBuilder::Build(const BuildInput& in, const TextTransform2D& transform,
	std::vector<MSDFTextVertexData>& outVertices, BuildOutput& out) const {

	out.drawIndexCount = 0;
	out.renderedGlyphCount = 0;
	out.bounds.min = Vector2::AnyInit(0.0f);
	out.bounds.max = Vector2::AnyInit(0.0f);
	if (!in.font || !in.codepoints) {
		return false;
	}

	const MSDFFont& font = *in.font;
	const std::vector<char32_t>& codepoints = *in.codepoints;

	// 頂点配列サイズ保証
	if (outVertices.size() < static_cast<size_t>(in.maxGlyphs) * 4) {
		outVertices.resize(static_cast<size_t>(in.maxGlyphs) * 4);
	}
	// 余りは0で埋め
	std::fill(outVertices.begin(), outVertices.end(), MSDFTextVertexData{});

	// pivot
	out.glyphPivots.assign(in.maxGlyphs, Vector2(0.0f, 0.0f));

	if (codepoints.empty()) {
		return false;
	}

	const MSDFMetrics& metrics = font.GetMetrics();

	// emサイズ、lineHeight補正
	float emSize = metrics.emSize;
	if (emSize <= 0.0f) emSize = 1.0f;
	float lineHeight = metrics.lineHeight;
	if (lineHeight <= 0.0f) {
		lineHeight = metrics.ascender - metrics.descender;
		if (lineHeight <= 0.0f) {
			lineHeight = emSize;
		}
	}

	// スケール計算
	const float scale = in.fontSizePx / emSize;
	const float invAtlasW = 1.0f / static_cast<float>(font.GetAtlasWidth());
	const float invAtlasH = 1.0f / static_cast<float>(font.GetAtlasHeight());

	// FixedAdvance解決
	const float fixedAdvancePx = ResolveFixedAdvance(font, in.spacing, codepoints, scale);

	//===============================================================================================
	// テキストボックス有り
	//===============================================================================================
	if (transform.enableTextBox) {

		// 行情報
		struct LineInfo {
			uint32_t beginGlyph;
			uint32_t endGlyph;
			float minX;
			float maxX;
		};

		// テキストボックス関連計算
		const float sx = (std::max)(1.0e-6f, std::abs(transform.sizeScale.x));
		const float sy = (std::max)(1.0e-6f, std::abs(transform.sizeScale.y));
		const float invSx = 1.0f / sx;
		const float invSy = 1.0f / sy;
		// テキストボックスサイズとパディングもスケール補正
		const float boxW = transform.textBoxSize.x * invSx;
		const float boxH = transform.textBoxSize.y * invSy;
		const float padX = transform.textBoxPadding.x * invSx;
		const float padY = transform.textBoxPadding.y * invSy;
		const float lineSpacingLocal = transform.lineSpacing * invSy;
		// 利用可能領域
		const float availW = (std::max)(0.0f, boxW - padX * 2.0f);
		const float availH = (std::max)(0.0f, boxH - padY * 2.0f);
		// ペン初期位置
		float penX = padX;
		float penY = padY - metrics.ascender * scale;

		uint32_t glyphCount = 0;
		char32_t prev = 0;
		// 行情報リスト
		std::vector<LineInfo> lines;
		LineInfo currentLine{};
		currentLine.beginGlyph = 0;
		currentLine.endGlyph = 0;
		currentLine.minX = 1.0e9f;
		currentLine.maxX = -1.0e9f;
		// 行確定ラムダ
		auto flushLine = [&]() {
			currentLine.endGlyph = glyphCount;
			if (currentLine.beginGlyph < currentLine.endGlyph) {
				lines.push_back(currentLine);
			}
			currentLine.beginGlyph = glyphCount;
			currentLine.minX = 1.0e9f;
			currentLine.maxX = -1.0e9f;
			};
		// 折り返し判定ラムダ
		auto willWrap = [&](float nextMaxX) -> bool {
			if (transform.wrapMode != TextWrapMode::CharWrap) {
				return false;
			}
			if (availW <= 0.0f) {
				return false;
			}
			if (penX <= padX) {
				return false;
			}
			float w = nextMaxX - padX;
			return (w > availW);
			};
		for (size_t i = 0; i < codepoints.size(); ++i) {

			const char32_t cp = codepoints[i];

			if (cp == U'\r') {
				continue;
			}

			if (cp == U'\n') {

				// 改行して次に進める
				flushLine();
				penX = padX;
				penY += (lineHeight * scale + lineSpacingLocal);
				prev = 0;

				// 高さオーバーなら打ち切り
				float lineTop, lineBottom;
				ComputeLineBoxTopBottomYDown(metrics, penY, scale, lineTop, lineBottom);
				if ((padY + availH) < lineBottom) {
					break;
				}
				continue;
			}

			const MSDFGlyph* glyph = font.FindGlyph(cp);
			const bool curFixed = ShouldUseFixedAdvance(cp, in.spacing);
			const bool prevFixed = (prev != 0) ? ShouldUseFixedAdvance(prev, in.spacing) : false;

			// カーニング
			if (prev != 0 && !(prevFixed || curFixed)) {
				penX += font.GetKerning(prev, cp) * scale;
			}
			prev = cp;

			const GlyphPlacement place = ComputeGlyphPlacement(*glyph, cp, scale, fixedAdvancePx, in.spacing);

			// bounds無し文字
			if (!glyph->planeBounds.has_value() || !glyph->atlasBounds.has_value()) {
				penX += place.advance + in.charSpacingPx;
				continue;
			}

			const MSDFPlaneBounds pb = glyph->planeBounds.value();
			const MSDFAtlasBounds ab = glyph->atlasBounds.value();

			Quad2D quad = MakeGlyphQuadYDown(penX + place.offset, penY, pb, scale);

			// 行幅判定
			float quadMinX = quad.x0;
			float quadMaxX = quad.x1;
			if (place.useFixed) {
				quadMinX = (std::min)(quadMinX, penX);
				quadMaxX = (std::max)(quadMaxX, penX + place.advance);
			}

			// 現在の行の範囲取得
			float curMinX = currentLine.minX;
			float curMaxX = currentLine.maxX;
			if (curMinX > 9.0e8f) {
				curMinX = quadMinX;
			}
			if (curMaxX < -9.0e8f) {
				curMaxX = quadMaxX;
			}

			// 次の範囲計算
			float nextMinX = (std::min)(curMinX, quadMinX);
			float nextMaxX = (std::max)(curMaxX, quadMaxX);

			// wrap
			if (willWrap(nextMaxX)) {

				if (padX < penX) {

					// 改行して次の行へ
					flushLine();
					penX = padX;
					penY += (lineHeight * scale + lineSpacingLocal);
					prev = 0;

					// 高さオーバーなら打ち切り
					float lineTop, lineBottom;
					ComputeLineBoxTopBottomYDown(metrics, penY, scale, lineTop, lineBottom);
					if ((padY + availH) < lineBottom) {
						break;
					}

					// 改行後の四角形再計算
					quad = MakeGlyphQuadYDown(penX + place.offset, penY, pb, scale);

					// 行幅判定
					quadMinX = quad.x0;
					quadMaxX = quad.x1;
					if (place.useFixed) {
						quadMinX = (std::min)(quadMinX, penX);
						quadMaxX = (std::max)(quadMaxX, penX + place.advance);
					}

					// 次の範囲計算
					nextMinX = quadMinX;
					nextMaxX = quadMaxX;
				}
			}

			// 高さオーバー判定
			if ((padY + availH) < quad.y1) {
				break;
			}

			// 書き込み
			const UVRect uv = MakeGlyphUV(ab, invAtlasW, invAtlasH);
			const uint32_t base = glyphCount * 4;
			WriteQuadVertices(&outVertices[base], quad, uv);
			out.glyphPivots[glyphCount] = QuadCenter(quad);
			currentLine.minX = (std::min)(currentLine.minX, quadMinX);
			currentLine.maxX = (std::max)(currentLine.maxX, quadMaxX);

			// 次へ
			++glyphCount;
			if (in.maxGlyphs <= glyphCount) {
				break;
			}
			// ペン進行
			penX += place.advance + in.charSpacingPx;
		}

		// 最終行確定
		flushLine();

		if (glyphCount == 0) {
			return false;
		}

		// 行揃え
		for (const auto& line : lines) {

			const float lineW = (line.maxX - line.minX);
			float targetMin = padX + (availW - lineW) * 0.5f;
			const float dx = targetMin - line.minX;

			for (uint32_t gi = line.beginGlyph; gi < line.endGlyph; ++gi) {

				// 横移動
				const uint32_t base = gi * 4;
				OffsetQuadVerticesX(&outVertices[base], dx);
				out.glyphPivots[gi].x += dx;
			}
		}

		// 縦揃え
		float contentMinY = 1.0e9f;
		float contentMaxY = -1.0e9f;

		for (uint32_t gi = 0; gi < glyphCount; ++gi) {

			const uint32_t base = gi * 4;
			for (uint32_t k = 0; k < 4; ++k) {

				contentMinY = (std::min)(contentMinY, outVertices[base + k].pos.y);
				contentMaxY = (std::max)(contentMaxY, outVertices[base + k].pos.y);
			}
		}

		float contentH = contentMaxY - contentMinY;
		float targetMinY = padY;
		if (transform.verticalAlign == TextVerticalAlign::Middle) {

			targetMinY = padY + (availH - contentH) * 0.5f;
		} else if (transform.verticalAlign == TextVerticalAlign::Bottom) {

			targetMinY = padY + (availH - contentH);
		}

		float dy = targetMinY - contentMinY;
		for (uint32_t gi = 0; gi < glyphCount; ++gi) {

			// 縦移動
			const uint32_t base = gi * 4;
			OffsetQuadVerticesY(&outVertices[base], dy);
			out.glyphPivots[gi].y += dy;
		}

		// 箱pivotを原点へ
		const float boxPivotX = transform.anchorPoint.x * boxW;
		const float boxPivotY = transform.anchorPoint.y * boxH;

		Vector2 minV(1.0e9f, 1.0e9f);
		Vector2 maxV(-1.0e9f, -1.0e9f);

		for (uint32_t gi = 0; gi < glyphCount; ++gi) {

			const uint32_t base = gi * 4;
			OffsetQuadVerticesXY(&outVertices[base], -boxPivotX, -boxPivotY);

			for (uint32_t k = 0; k < 4; ++k) {

				minV.x = (std::min)(minV.x, outVertices[base + k].pos.x);
				minV.y = (std::min)(minV.y, outVertices[base + k].pos.y);
				maxV.x = (std::max)(maxV.x, outVertices[base + k].pos.x);
				maxV.y = (std::max)(maxV.y, outVertices[base + k].pos.y);
			}

			out.glyphPivots[gi].x -= boxPivotX;
			out.glyphPivots[gi].y -= boxPivotY;
		}

		// bounds設定
		out.bounds.min = minV;
		out.bounds.max = maxV;
		// 描画情報設定
		out.renderedGlyphCount = glyphCount;
		out.drawIndexCount = glyphCount * 6;
		return true;
	}

	//===============================================================================================
	// テキストボックス無し
	//===============================================================================================

	float penX = 0.0f;
	float penY = 0.0f;

	Vector2 minV(1.0e9f, 1.0e9f);
	Vector2 maxV(-1.0e9f, -1.0e9f);

	uint32_t glyphCount = 0;
	char32_t prev = 0;

	for (size_t i = 0; i < codepoints.size(); ++i) {

		const char32_t cp = codepoints[i];

		if (cp == U'\r') {
			continue;
		}

		if (cp == U'\n') {
			penX = 0.0f;
			penY += lineHeight * scale;
			prev = 0;
			continue;
		}

		// グリフ取得
		const MSDFGlyph* glyph = font.FindGlyph(cp);
		const bool curFixed = ShouldUseFixedAdvance(cp, in.spacing);
		const bool prevFixed = (prev != 0) ? ShouldUseFixedAdvance(prev, in.spacing) : false;
		// カーニング
		if (prev != 0 && !(prevFixed || curFixed)) {
			penX += font.GetKerning(prev, cp) * scale;
		}
		prev = cp;

		// グリフ配置計算
		const GlyphPlacement place = ComputeGlyphPlacement(*glyph, cp, scale, fixedAdvancePx, in.spacing);
		if (!glyph->planeBounds.has_value() || !glyph->atlasBounds.has_value()) {
			penX += place.advance + in.charSpacingPx;
			continue;
		}

		// グリフ情報取得
		const MSDFPlaneBounds pb = glyph->planeBounds.value();
		const MSDFAtlasBounds ab = glyph->atlasBounds.value();
		const Quad2D quad = MakeGlyphQuadYDown(penX + place.offset, penY, pb, scale);
		const UVRect uv = MakeGlyphUV(ab, invAtlasW, invAtlasH);

		// 頂点書き込み
		const uint32_t base = glyphCount * 4;
		WriteQuadVertices(&outVertices[base], quad, uv);
		out.glyphPivots[glyphCount] = QuadCenter(quad);

		// bounds計算
		float quadMinX = quad.x0;
		float quadMaxX = quad.x1;
		if (place.useFixed) {
			quadMinX = (std::min)(quadMinX, penX);
			quadMaxX = (std::max)(quadMaxX, penX + place.advance);
		}
		minV.x = (std::min)(minV.x, quadMinX);
		minV.y = (std::min)(minV.y, quad.y0);
		maxV.x = (std::max)(maxV.x, quadMaxX);
		maxV.y = (std::max)(maxV.y, quad.y1);

		// 次へ
		++glyphCount;
		if (glyphCount >= in.maxGlyphs) {
			break;
		}
		// ペン進行
		penX += place.advance + in.charSpacingPx;
	}

	if (glyphCount == 0) {
		return false;
	}

	out.bounds.min = minV;
	out.bounds.max = maxV;

	// anchorPointに合わせて原点補正
	const float w = out.bounds.Width();
	const float h = out.bounds.Height();
	const float pivotX = out.bounds.min.x + transform.anchorPoint.x * w;
	const float pivotY = out.bounds.min.y + transform.anchorPoint.y * h;

	for (uint32_t gi = 0; gi < glyphCount; ++gi) {

		// 移動
		const uint32_t base = gi * 4;
		OffsetQuadVerticesXY(&outVertices[base], -pivotX, -pivotY);

		out.glyphPivots[gi].x -= pivotX;
		out.glyphPivots[gi].y -= pivotY;
	}

	// bounds補正
	out.bounds.min.x -= pivotX;
	out.bounds.min.y -= pivotY;
	out.bounds.max.x -= pivotX;
	out.bounds.max.y -= pivotY;
	// 描画情報設定
	out.renderedGlyphCount = glyphCount;
	out.drawIndexCount = glyphCount * 6;
	return true;
}
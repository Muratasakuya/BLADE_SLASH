#include "MSDFText.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	MSDFText classMethods
//============================================================================

MSDFText::MSDFText(ID3D12Device* device, Asset* asset, const MSDFFont* font, uint32_t maxGlyphs) {

	device_ = device;
	asset_ = asset;
	font_ = font;

	maxGlyphs_ = maxGlyphs;

	// バッファ作成、頂点数は最大文字数×4、インデックス数は最大文字数×6
	vertexBuffer_.CreateBuffer(device_, maxGlyphs_ * 4);
	indexBuffer_.CreateBuffer(device_, maxGlyphs_ * 6);

	// インデックスバッファ構築
	BuildIndexBuffer();
	// デフォルト文字列設定
	SetText("abcABC");
}

void MSDFText::SetText(const std::string& utf8) {

	textUtf8_ = utf8;
	codepoints_ = Algorithm::Utf8ToCodepoints(textUtf8_);
	dirtyMesh_ = true;

	// エディター用入力欄も更新
	inputText_.inputText = textUtf8_;
	inputText_.CopyToBuffer();
}

void MSDFText::SetText(int32_t value) {

	SetText(std::to_string(value));
}

void MSDFText::SetText(float value, int32_t precision) {

	// 文字列化
	std::ostringstream oss;
	oss.setf(std::ios::fixed);
	oss << std::setprecision(precision) << value;
	SetText(oss.str());
}

void MSDFText::SetFontSizePx(float pixel) {

	fontSize_ = pixel;
	dirtyMesh_ = true;
}

void MSDFText::EnsureCapacity(uint32_t glyphCount) {

	// 十分な容量があれば何もしない
	if (glyphCount <= maxGlyphs_) {
		return;
	}

	// 容量拡張
	uint32_t newMaxGlyphs = maxGlyphs_;
	while (newMaxGlyphs < glyphCount) {
		newMaxGlyphs *= 2;
	}

	// バッファ再作成
	maxGlyphs_ = newMaxGlyphs;
	vertexBuffer_.CreateBuffer(device_, maxGlyphs_ * 4);
	indexBuffer_.CreateBuffer(device_, maxGlyphs_ * 6);
	BuildIndexBuffer();
	dirtyMesh_ = true;
	glyphPivots_.resize(maxGlyphs_);
}

void MSDFText::BuildIndexBuffer() {

	// インデックスデータ作成
	std::vector<uint32_t> indices(maxGlyphs_ * 6);

	for (uint32_t glyphIndex = 0; glyphIndex < maxGlyphs_; ++glyphIndex) {

		const uint32_t baseVertexIndex = glyphIndex * 4;
		const uint32_t baseIndexIndex = glyphIndex * 6;

		// 頂点並び
		indices[baseIndexIndex] = baseVertexIndex;
		indices[baseIndexIndex + 1] = baseVertexIndex + 1;
		indices[baseIndexIndex + 2] = baseVertexIndex + 2;

		indices[baseIndexIndex + 3] = baseVertexIndex + 2;
		indices[baseIndexIndex + 4] = baseVertexIndex + 1;
		indices[baseIndexIndex + 5] = baseVertexIndex + 3;
	}
	indexBuffer_.TransferData(indices);
}

void MSDFText::UpdateVertex(const TextTransform2D& transform) {

	// 変更がなければ何もしない
	if (!dirtyMesh_ && !IsDirtyTransform(transform)) {
		return;
	}

	// 頂点バッファの容量を必要に応じて拡張
	EnsureCapacity(static_cast<uint32_t>(codepoints_.size()));
	// メッシュ再構築
	RebuildMeshCPU(transform);
	dirtyMesh_ = false;
}

void MSDFText::RebuildMeshCPU(const TextTransform2D& transform) {

	drawIndexCount_ = 0;

	// 最大数分の頂点領域
	std::vector<MSDFTextVertexData> vertices(maxGlyphs_ * 4);

	// 文字が無ければ何もしない
	if (codepoints_.empty()) {
		vertexBuffer_.TransferData(vertices);
		drawIndexCount_ = 0;
		renderedGlyphCount_ = 0;
		bounds_.min = Vector2::AnyInit(0.0f);
		bounds_.max = Vector2::AnyInit(0.0f);
		return;
	}

	// メトリクス取得
	const MSDFMetrics& metrics = font_->GetMetrics();

	float emSize = metrics.emSize;
	if (emSize <= 0.0f) {
		emSize = 1.0f;
	}

	float lineHeight = metrics.lineHeight;
	if (lineHeight <= 0.0f) {
		lineHeight = (metrics.ascender - metrics.descender);
		if (lineHeight <= 0.0f) {
			lineHeight = emSize;
		}
	}

	// スケール計算
	const float scale = fontSize_ / emSize;

	// ピボット初期化、描画する分だけ確保
	glyphPivots_.assign(maxGlyphs_, Vector2(0.0f, 0.0f));

	//===============================================================================================
	// テキストボックス処理
	//===============================================================================================
	if (transform.enableTextBox) {

		struct LineInfo {

			uint32_t beginGlyph;
			uint32_t endGlyph;
			float minX;
			float maxX;
		};

		// 箱情報
		const float boxW = transform.textBoxSize.x;
		const float boxH = transform.textBoxSize.y;
		const float padX = transform.textBoxPadding.x;
		const float padY = transform.textBoxPadding.y;

		// 利用可能領域
		const float availW = (std::max)(0.0f, boxW - padX * 2.0f);
		const float availH = (std::max)(0.0f, boxH - padY * 2.0f);
		float penX = padX;
		float penY = padY + metrics.ascender * scale;

		// 実際にクアッドを作った数
		uint32_t glyphCount = 0;
		char32_t prev = 0;

		std::vector<LineInfo> lines;
		LineInfo currentLine{};
		currentLine.beginGlyph = 0;
		currentLine.endGlyph = 0;
		currentLine.minX = 1.0e9f;
		currentLine.maxX = -1.0e9f;

		auto flushLine = [&]() {
			currentLine.endGlyph = glyphCount;
			if (currentLine.beginGlyph < currentLine.endGlyph) {
				lines.push_back(currentLine);
			}
			currentLine.beginGlyph = glyphCount;
			currentLine.minX = 1.0e9f;
			currentLine.maxX = -1.0e9f;
			};
		auto willWrap = [&](float nextMinX, float nextMaxX) -> bool {
			if (transform.wrapMode != TextWrapMode::CharWrap) {
				return false;
			}
			if (availW <= 0.0f) {
				return false;
			}
			// 行頭では基本折り返さない
			if (penX <= padX) {
				return false;
			}
			return (nextMaxX - nextMinX) > availW;
			};
		auto safeMin = [&](float a, float b) { return (std::min)(a, b); };
		auto safeMax = [&](float a, float b) { return (std::max)(a, b); };
		for (size_t i = 0; i < codepoints_.size(); ++i) {

			const char32_t codepoint = codepoints_[i];

			// CRは無視
			if (codepoint == U'\r') {
				continue;
			}

			// 明示改行
			if (codepoint == U'\n') {

				flushLine();

				penX = padX;
				penY += (lineHeight * scale + transform.lineSpacing);
				prev = 0;

				// 高さオーバーなら打ち切り
				if ((padY + availH) < (penY - metrics.ascender * scale)) {
					break;
				}
				continue;
			}

			const MSDFGlyph* glyph = font_->FindGlyph(codepoint);
			if (!glyph) {
				continue;
			}

			// カーニング
			if (prev != 0) {
				penX += font_->GetKerning(prev, codepoint) * scale;
			}
			prev = codepoint;

			const MSDFPlaneBounds planeBounds = glyph->planeBounds.value();
			const MSDFAtlasBounds atlasBounds = glyph->atlasBounds.value();

			float x0 = penX + planeBounds.left * scale;
			float x1 = penX + planeBounds.right * scale;
			float y0 = penY + (-planeBounds.top) * scale;
			float y1 = penY + (-planeBounds.bottom) * scale;

			// 行の現在min/max
			float curMinX = currentLine.minX;
			float curMaxX = currentLine.maxX;
			if (curMinX > 9.0e8f) {
				curMinX = x0;
			}
			if (curMaxX < -9.0e8f) {
				curMaxX = x1;
			}

			// この文字を置いたときの行幅を予測
			float nextMinX = safeMin(curMinX, x0);
			float nextMaxX = safeMax(curMaxX, x1);

			// 折り返し
			if (willWrap(nextMinX, nextMaxX)) {
				if ((padX + availW) < x1 && padX < penX) {

					flushLine();

					penX = padX;
					penY += (lineHeight * scale + transform.lineSpacing);
					prev = 0;

					if ((padY + availH) < (penY - metrics.ascender * scale)) {
						break;
					}

					// 改行後に再計算
					x0 = penX + planeBounds.left * scale;
					x1 = penX + planeBounds.right * scale;
					y0 = penY + (-planeBounds.top) * scale;
					y1 = penY + (-planeBounds.bottom) * scale;
					// 改行後の予測値を作り直し
					nextMinX = x0;
					nextMaxX = x1;
				}
			}

			// 高さオーバーなら打ち切り
			if ((padY + availH) < y1) {
				break;
			}

			// UV
			const float invAtlasWidth = 1.0f / static_cast<float>(font_->GetAtlasWidth());
			const float invAtlasHeight = 1.0f / static_cast<float>(font_->GetAtlasHeight());

			float u0 = static_cast<float>(atlasBounds.left) * invAtlasWidth;
			float u1 = static_cast<float>(atlasBounds.right) * invAtlasWidth;
			float v0 = static_cast<float>(atlasBounds.top) * invAtlasHeight;
			float v1 = static_cast<float>(atlasBounds.bottom) * invAtlasHeight;

			if (v1 < v0) {
				(std::swap)(v0, v1);
			}

			const uint32_t base = glyphCount * 4;
			vertices[base].pos = Vector2(x0, y0);
			vertices[base].texcoord = Vector2(u0, v1);

			vertices[base + 1].pos = Vector2(x0, y1);
			vertices[base + 1].texcoord = Vector2(u0, v0);

			vertices[base + 2].pos = Vector2(x1, y0);
			vertices[base + 2].texcoord = Vector2(u1, v1);

			vertices[base + 3].pos = Vector2(x1, y1);
			vertices[base + 3].texcoord = Vector2(u1, v0);

			glyphPivots_[glyphCount] = Vector2((x0 + x1) * 0.5f, (y0 + y1) * 0.5f);

			// 行のmin/max更新
			currentLine.minX = (std::min)(currentLine.minX, x0);
			currentLine.maxX = (std::max)(currentLine.maxX, x1);

			++glyphCount;
			if (maxGlyphs_ <= glyphCount) {
				break;
			}

			// 次の文字へ
			penX += glyph->advance * scale + charSpacing_;
		}

		// 最終行
		flushLine();

		// 文字が一つも無い
		if (glyphCount == 0) {
			vertexBuffer_.TransferData(vertices);
			drawIndexCount_ = 0;
			renderedGlyphCount_ = 0;
			bounds_.min = Vector2::AnyInit(0.0f);
			bounds_.max = Vector2::AnyInit(0.0f);
			return;
		}

		// 行揃え
		for (const auto& line : lines) {

			const float lineW = (line.maxX - line.minX);

			float targetMin = padX + (availW - lineW) * 0.5f;
			const float dx = targetMin - line.minX;
			for (uint32_t gi = line.beginGlyph; gi < line.endGlyph; ++gi) {

				const uint32_t base = gi * 4;

				vertices[base].pos.x += dx;
				vertices[base + 1].pos.x += dx;
				vertices[base + 2].pos.x += dx;
				vertices[base + 3].pos.x += dx;

				glyphPivots_[gi].x += dx;
			}
		}

		// 縦揃え
		float contentMinY = 1.0e9f;
		float contentMaxY = -1.0e9f;
		for (uint32_t gi = 0; gi < glyphCount; ++gi) {
			const uint32_t base = gi * 4;

			contentMinY = (std::min)(contentMinY, vertices[base + 0].pos.y);
			contentMinY = (std::min)(contentMinY, vertices[base + 1].pos.y);
			contentMinY = (std::min)(contentMinY, vertices[base + 2].pos.y);
			contentMinY = (std::min)(contentMinY, vertices[base + 3].pos.y);

			contentMaxY = (std::max)(contentMaxY, vertices[base + 0].pos.y);
			contentMaxY = (std::max)(contentMaxY, vertices[base + 1].pos.y);
			contentMaxY = (std::max)(contentMaxY, vertices[base + 2].pos.y);
			contentMaxY = (std::max)(contentMaxY, vertices[base + 3].pos.y);
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

			const uint32_t base = gi * 4;

			vertices[base].pos.y += dy;
			vertices[base + 1].pos.y += dy;
			vertices[base + 2].pos.y += dy;
			vertices[base + 3].pos.y += dy;
			glyphPivots_[gi].y += dy;
		}

		// 箱のアンカー位置を原点にする
		const float boxPivotX = transform.anchorPoint.x * boxW;
		const float boxPivotY = transform.anchorPoint.y * boxH;

		Vector2 minV(1.0e9f, 1.0e9f);
		Vector2 maxV(-1.0e9f, -1.0e9f);

		for (uint32_t gi = 0; gi < glyphCount; ++gi) {

			const uint32_t base = gi * 4;

			for (uint32_t k = 0; k < 4; ++k) {
				vertices[base + k].pos.x -= boxPivotX;
				vertices[base + k].pos.y -= boxPivotY;

				minV.x = (std::min)(minV.x, vertices[base + k].pos.x);
				minV.y = (std::min)(minV.y, vertices[base + k].pos.y);
				maxV.x = (std::max)(maxV.x, vertices[base + k].pos.x);
				maxV.y = (std::max)(maxV.y, vertices[base + k].pos.y);
			}
			glyphPivots_[gi].x -= boxPivotX;
			glyphPivots_[gi].y -= boxPivotY;
		}

		// bounds
		bounds_.min = minV;
		bounds_.max = maxV;

		// 描画グリフ数
		renderedGlyphCount_ = glyphCount;

		// 転送
		vertexBuffer_.TransferData(vertices);
		drawIndexCount_ = glyphCount * 6;
		return;
	}

	//===============================================================================================
	// テキストボックス無し処理
	//===============================================================================================

	float penX = 0.0f;
	float penY = 0.0f;

	Vector2 minValue(1.0e9f, 1.0e9f);
	Vector2 maxValue(-1.0e9f, -1.0e9f);

	uint32_t glyphCount = 0;
	char32_t prevCodepoint = 0;

	for (size_t i = 0; i < codepoints_.size(); ++i) {

		const char32_t codepoint = codepoints_[i];

		// CRは無視
		if (codepoint == U'\r') {
			continue;
		}

		// 改行
		if (codepoint == U'\n') {
			penX = 0.0f;
			penY += lineHeight * scale;
			prevCodepoint = 0;
			continue;
		}

		const MSDFGlyph* glyph = font_->FindGlyph(codepoint);
		if (!glyph) {
			continue;
		}

		// カーニング
		if (prevCodepoint != 0) {
			penX += font_->GetKerning(prevCodepoint, codepoint) * scale;
		}
		prevCodepoint = codepoint;

		if (!glyph->planeBounds.has_value() || !glyph->atlasBounds.has_value()) {
			penX += glyph->advance * scale + charSpacing_;
			continue;
		}

		const MSDFPlaneBounds planeBounds = glyph->planeBounds.value();
		const MSDFAtlasBounds atlasBounds = glyph->atlasBounds.value();

		const float x0 = penX + planeBounds.left * scale;
		const float x1 = penX + planeBounds.right * scale;
		const float y0 = penY + (-planeBounds.top) * scale;
		const float y1 = penY + (-planeBounds.bottom) * scale;

		// pivot
		glyphPivots_[glyphCount] = Vector2((x0 + x1) * 0.5f, (y0 + y1) * 0.5f);

		// UV
		const float invAtlasWidth = 1.0f / static_cast<float>(font_->GetAtlasWidth());
		const float invAtlasHeight = 1.0f / static_cast<float>(font_->GetAtlasHeight());

		float u0 = static_cast<float>(atlasBounds.left) * invAtlasWidth;
		float u1 = static_cast<float>(atlasBounds.right) * invAtlasWidth;
		float v0 = static_cast<float>(atlasBounds.top) * invAtlasHeight;
		float v1 = static_cast<float>(atlasBounds.bottom) * invAtlasHeight;

		if (v1 < v0) {
			(std::swap)(v0, v1);
		}

		const uint32_t base = glyphCount * 4;

		vertices[base].pos = Vector2(x0, y0);
		vertices[base].texcoord = Vector2(u0, v1);

		vertices[base + 1].pos = Vector2(x0, y1);
		vertices[base + 1].texcoord = Vector2(u0, v0);

		vertices[base + 2].pos = Vector2(x1, y0);
		vertices[base + 2].texcoord = Vector2(u1, v1);

		vertices[base + 3].pos = Vector2(x1, y1);
		vertices[base + 3].texcoord = Vector2(u1, v0);

		// bounds更新
		minValue.x = (std::min)(minValue.x, x0);
		minValue.y = (std::min)(minValue.y, y0);
		maxValue.x = (std::max)(maxValue.x, x1);
		maxValue.y = (std::max)(maxValue.y, y1);

		++glyphCount;
		if (glyphCount >= maxGlyphs_) {
			break;
		}

		penX += glyph->advance * scale + charSpacing_;
	}

	// 文字が無い
	if (glyphCount == 0) {
		vertexBuffer_.TransferData(vertices);
		drawIndexCount_ = 0;
		renderedGlyphCount_ = 0;
		bounds_.min = Vector2::AnyInit(0.0f);
		bounds_.max = Vector2::AnyInit(0.0f);
		return;
	}

	// bounds確定
	bounds_.min = minValue;
	bounds_.max = maxValue;

	// アンカーポイント基準に頂点移動
	float width = bounds_.Width();
	float height = bounds_.Height();
	float pivotX = bounds_.min.x + transform.anchorPoint.x * width;
	float pivotY = bounds_.min.y + transform.anchorPoint.y * height;
	for (uint32_t gi = 0; gi < glyphCount; ++gi) {

		const uint32_t base = gi * 4;
		for (uint32_t k = 0; k < 4; ++k) {

			vertices[base + k].pos.x -= pivotX;
			vertices[base + k].pos.y -= pivotY;
		}
		glyphPivots_[gi].x -= pivotX;
		glyphPivots_[gi].y -= pivotY;
	}

	// 描画グリフ数
	renderedGlyphCount_ = glyphCount;

	// boundsも移動
	bounds_.min.x -= pivotX;
	bounds_.min.y -= pivotY;
	bounds_.max.x -= pivotX;
	bounds_.max.y -= pivotY;

	// 転送
	vertexBuffer_.TransferData(vertices);
	drawIndexCount_ = glyphCount * 6;
}

bool MSDFText::IsDirtyTransform(const TextTransform2D& transform) const {

	bool dirty = false;
	dirty |= (prevTransform_.translation != transform.translation);
	dirty |= (prevTransform_.enableTextBox != transform.enableTextBox);
	dirty |= (prevTransform_.textBoxSize != transform.textBoxSize);
	dirty |= (prevTransform_.textBoxPadding != transform.textBoxPadding);
	dirty |= (prevTransform_.lineSpacing != transform.lineSpacing);
	dirty |= (prevTransform_.wrapMode != transform.wrapMode);
	return dirty;
}

void MSDFText::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	inputText_.inputText = textUtf8_;
	if (ImGuiHelper::InputText("Text", inputText_)) {
		SetText(inputText_.inputText);
	}
	ImGui::Text("Glyphs: %zu", codepoints_.size());

	ImGui::SeparatorText("Parameters");

	if (ImGui::DragFloat("fontSizePx", &fontSize_, 0.1f)) {
		dirtyMesh_ = true;
	}
	if (ImGui::DragFloat("charSpacing", &charSpacing_, 0.01f)) {
		dirtyMesh_ = true;
	}

	ImGui::PopItemWidth();
}
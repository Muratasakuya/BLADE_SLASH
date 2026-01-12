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

	// アンカー位置変更をチェック
	bool anchorChanged = (prevAnchorPoint_ != transform.anchorPoint);
	prevAnchorPoint_ = transform.anchorPoint;

	// 変更がなければ何もしない
	if (!dirtyMesh_ && !anchorChanged) {
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

	// 初期bounds
	bounds_.min = Vector2(0.0f, 0.0f);
	bounds_.max = Vector2(0.0f, 0.0f);

	std::vector<MSDFTextVertexData> vertices(maxGlyphs_ * 4);
	// 文字が無ければ何もしない
	if (codepoints_.empty()) {

		vertexBuffer_.TransferData(vertices);
		drawIndexCount_ = 0;
		return;
	}

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

	float penX = 0.0f;
	float penY = 0.0f;
	Vector2 minValue(1.0e9f, 1.0e9f);
	Vector2 maxValue(-1.0e9f, -1.0e9f);
	uint32_t glyphCount = 0;
	char32_t prevCodepoint = 0;
	glyphPivots_.assign(maxGlyphs_, Vector2(0.0f, 0.0f));
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

		// カーニング適用
		if (prevCodepoint != 0) {
			penX += font_->GetKerning(prevCodepoint, codepoint) * scale;
		}
		prevCodepoint = codepoint;

		if (!glyph->planeBounds.has_value() || !glyph->atlasBounds.has_value()) {

			penX += glyph->advance * scale + charSpacing_;
			continue;
		}

		// 頂点計算
		const MSDFPlaneBounds planeBounds = glyph->planeBounds.value();
		const MSDFAtlasBounds atlasBounds = glyph->atlasBounds.value();

		// 左上、右下座標計算
		const float x0 = penX + planeBounds.left * scale;
		const float x1 = penX + planeBounds.right * scale;
		const float y0 = penY + (-planeBounds.top) * scale;    // 上
		const float y1 = penY + (-planeBounds.bottom) * scale; // 下
		// ピボット計算
		glyphPivots_[glyphCount] = Vector2((x0 + x1) * 0.5f, (y0 + y1) * 0.5f);

		// UV計算
		const float invAtlasWidth = 1.0f / static_cast<float>(font_->GetAtlasWidth());
		const float invAtlasHeight = 1.0f / static_cast<float>(font_->GetAtlasHeight());
		float u0 = static_cast<float>(atlasBounds.left) * invAtlasWidth;
		float u1 = static_cast<float>(atlasBounds.right) * invAtlasWidth;
		float v0 = static_cast<float>(atlasBounds.top) * invAtlasHeight;
		float v1 = static_cast<float>(atlasBounds.bottom) * invAtlasHeight;

		// 上下が逆のケース保険
		if (v1 < v0) {
			std::swap(v0, v1);
		}

		// 頂点データ設定
		const uint32_t baseVertexIndex = glyphCount * 4;
		// 左下
		vertices[baseVertexIndex].pos = Vector2(x0, y0);
		vertices[baseVertexIndex].texcoord = Vector2(u0, v1);
		// 左上
		vertices[baseVertexIndex + 1].pos = Vector2(x0, y1);
		vertices[baseVertexIndex + 1].texcoord = Vector2(u0, v0);
		// 右下
		vertices[baseVertexIndex + 2].pos = Vector2(x1, y0);
		vertices[baseVertexIndex + 2].texcoord = Vector2(u1, v1);
		// 右上
		vertices[baseVertexIndex + 3].pos = Vector2(x1, y1);
		vertices[baseVertexIndex + 3].texcoord = Vector2(u1, v0);

		// bounds更新
		minValue.x = (std::min)(minValue.x, x0);
		minValue.y = (std::min)(minValue.y, y0);
		maxValue.x = (std::max)(maxValue.x, x1);
		maxValue.y = (std::max)(maxValue.y, y1);

		++glyphCount;
		if (glyphCount >= maxGlyphs_) {
			break;
		}
		// advance + 追加の文字間隔
		penX += glyph->advance * scale + charSpacing_;
	}

	// 文字が無ければ何もしない
	if (glyphCount == 0) {

		vertexBuffer_.TransferData(vertices);
		drawIndexCount_ = 0;
		bounds_.min = Vector2(0.0f, 0.0f);
		bounds_.max = Vector2(0.0f, 0.0f);
		return;
	}

	// bounds確定
	bounds_.min = minValue;
	bounds_.max = maxValue;

	// アンカーポイント基準に頂点移動
	const float width = bounds_.Width();
	const float height = bounds_.Height();
	const float pivotX = bounds_.min.x + transform.anchorPoint.x * width;
	const float pivotY = bounds_.min.y + transform.anchorPoint.y * height;

	for (uint32_t gi = 0; gi < glyphCount; ++gi) {

		const uint32_t baseVertexIndex = gi * 4;
		for (uint32_t k = 0; k < 4; ++k) {

			vertices[baseVertexIndex + k].pos.x -= pivotX;
			vertices[baseVertexIndex + k].pos.y -= pivotY;
		}
		// ピボットも移動
		glyphPivots_[gi].x -= pivotX;
		glyphPivots_[gi].y -= pivotY;
	}

	// 描画されるグリフ数確定
	renderedGlyphCount_ = glyphCount;
	// boundsも移動
	bounds_.min.x -= pivotX;
	bounds_.min.y -= pivotY;
	bounds_.max.x -= pivotX;
	bounds_.max.y -= pivotY;

	// 頂点バッファ転送
	vertexBuffer_.TransferData(vertices);
	drawIndexCount_ = glyphCount * 6;
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
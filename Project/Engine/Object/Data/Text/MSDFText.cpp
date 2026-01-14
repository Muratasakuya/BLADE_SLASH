#include "MSDFText.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Utility/Helper/Algorithm.h>

// テキスト構築
#include <Engine/Object/Data/Text/Generator/ClockTextGenerator.h>

// c++
#include <algorithm>
#include <array>
#include <limits>

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

	// 試運転
	textGenerator_ = std::make_unique<ClockTextGenerator>();
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
		indices[baseIndexIndex + 0] = baseVertexIndex + 0;
		indices[baseIndexIndex + 1] = baseVertexIndex + 2;
		indices[baseIndexIndex + 2] = baseVertexIndex + 1;

		indices[baseIndexIndex + 3] = baseVertexIndex + 2;
		indices[baseIndexIndex + 4] = baseVertexIndex + 3;
		indices[baseIndexIndex + 5] = baseVertexIndex + 1;
	}
	indexBuffer_.TransferData(indices);
}

void MSDFText::UpdateVertex(const TextTransform2D& transform) {

	// ジェネレーターから文字列更新
	UpdateTextFromGenerator();

	// 変更がなければ何もしない
	if (!dirtyMesh_ && !IsDirtyTransform(transform)) {
		return;
	}

	// 頂点バッファの容量を必要に応じて拡張
	EnsureCapacity(static_cast<uint32_t>(codepoints_.size()));

	// メッシュ構築
	std::vector<MSDFTextVertexData> vertices(maxGlyphs_ * 4);
	MSDFTextMeshBuilder::BuildInput in{};
	in.font = font_;
	in.codepoints = &codepoints_;
	in.fontSizePx = fontSize_;
	in.charSpacingPx = charSpacing_;
	in.spacing = spacing_;
	in.maxGlyphs = maxGlyphs_;

	// ビルド実行
	MSDFTextMeshBuilder::BuildOutput out{};
	bool built = meshBuilder_.Build(in, transform, vertices, out);

	// 何も描けない場合
	if (!built) {

		// 頂点バッファを空で更新
		vertexBuffer_.TransferData(vertices);
		drawIndexCount_ = 0;
		renderedGlyphCount_ = 0;
		bounds_.min = Vector2::AnyInit(0.0f);
		bounds_.max = Vector2::AnyInit(0.0f);
		prevTransform_ = transform;
		dirtyMesh_ = false;
		return;
	}

	// 頂点バッファ更新
	vertexBuffer_.TransferData(vertices);

	// 描画情報更新
	drawIndexCount_ = out.drawIndexCount;
	renderedGlyphCount_ = out.renderedGlyphCount;
	bounds_ = out.bounds;
	glyphPivots_ = std::move(out.glyphPivots);
	prevTransform_ = transform;
	dirtyMesh_ = false;
}

void MSDFText::UpdateTextFromGenerator() {

	// ジェネレーターが無ければ何もしない
	if (!textGenerator_) {
		return;
	}

	// 文字列生成
	std::string outString{};
	if (textGenerator_->Generate(outString)) {

		// 文字列変更
		SetText(outString);
	}
}

bool MSDFText::IsDirtyTransform(const TextTransform2D& transform) const {

	bool dirty = false;
	dirty |= (prevTransform_.translation != transform.translation);
	dirty |= (prevTransform_.enableTextBox != transform.enableTextBox);
	dirty |= (prevTransform_.textBoxSize != transform.textBoxSize);
	dirty |= (prevTransform_.textBoxPadding != transform.textBoxPadding);
	dirty |= (prevTransform_.lineSpacing != transform.lineSpacing);
	dirty |= (prevTransform_.wrapMode != transform.wrapMode);
	dirty |= (prevTransform_.anchorPoint != transform.anchorPoint);
	return dirty;
}

void MSDFText::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	ImGui::SeparatorText("Text");

	if (ImGuiHelper::InputText("##", inputText_)) {
		// generator 使用時は editor 書き換えを禁止したい場合もあるので
		// 必要ならここで if (!textGenerator_) を入れてください
		SetText(inputText_.inputText);
	}

	if (ImGui::DragFloat("previewFloat", &previewFloat_, 0.01f)) {
		SetText(previewFloat_, previewPrecision_);
	}
	if (ImGui::DragInt("previewPrecision", &previewPrecision_, 1, 0, 128)) {
		SetText(previewFloat_, previewPrecision_);
	}

	ImGui::Text("GlyphCount: %zu / %zu", codepoints_.size(), maxGlyphs_);
	ImGui::Text("RenderedGlyphCount: %u", renderedGlyphCount_);

	ImGui::SeparatorText("Style");

	if (ImGui::DragFloat("fontSizePx", &fontSize_, 0.1f)) {
		dirtyMesh_ = true;
	}
	if (ImGui::DragFloat("charSpacingPx", &charSpacing_, 0.01f)) {
		dirtyMesh_ = true;
	}

	ImGui::SeparatorText("SpacingParams");

	{
		int mode = static_cast<int>(spacing_.mode);
		if (ImGui::Combo("mode", &mode, "Tight\0FixedAdvance\0")) {
			spacing_.mode = static_cast<TextSpacingMode>(mode);
			dirtyMesh_ = true;
		}
		if (ImGui::Checkbox("applyOnlyToNumericSet", &spacing_.applyOnlyToNumericSet)) {
			dirtyMesh_ = true;
		}
		if (ImGui::DragFloat("fixedAdvancePx(0=auto)", &spacing_.fixedAdvance, 0.1f, 0.0f, 10000.0f)) {
			dirtyMesh_ = true;
		}
		if (ImGui::SliderFloat("cellAlign", &spacing_.cellAlign, 0.0f, 1.0f)) {
			dirtyMesh_ = true;
		}
	}

	ImGui::PopItemWidth();
}

void MSDFText::SetRenderResources(uint32_t objectId) {

	// 必要なデータを取得
	ObjectManager* objectManager = ObjectManager::GetInstance();
	auto* transform = objectManager->GetData<TextTransform2D>(objectId);
	auto* material = objectManager->GetData<MSDFTextMaterial>(objectId);

	// リソース追加
	AddRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::Constant,
		.rootParamIndex = 1,
		.bufferAddress = transform->GetBuffer().GetResource()->GetGPUVirtualAddress(),
		.bufferHandle = D3D12_GPU_DESCRIPTOR_HANDLE{},
		});
	AddRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::Structured,
		.rootParamIndex = 2,
		.bufferAddress = transform->GetCharMatrixBuffer().GetResource()->GetGPUVirtualAddress(),
		.bufferHandle = D3D12_GPU_DESCRIPTOR_HANDLE{},
		});
	AddRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::TextureGPU,
		.rootParamIndex = 3,
		.bufferAddress = 0,
		.bufferHandle = font_->GetAtlasGPUHandle(),
		});
	AddRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::Constant,
		.rootParamIndex = 4,
		.bufferAddress = material->GetBuffer().GetResource()->GetGPUVirtualAddress(),
		.bufferHandle = D3D12_GPU_DESCRIPTOR_HANDLE{},
		});
}

void MSDFText::DrawCommand(ID3D12GraphicsCommandList6* commandList) {

	// 文字が無ければ描画しない
	if (GetGlyphCount() == 0) {
		return;
	}

	// 頂点バッファ設定
	commandList->IASetVertexBuffers(0, 1, &vertexBuffer_.GetVertexBufferView());
	// インデックスバッファ設定
	commandList->IASetIndexBuffer(&indexBuffer_.GetIndexBufferView());
	// 描画リソース
	BaseCanvas::SetRenderResourceCommand(commandList);
	// 描画コマンド
	commandList->DrawIndexedInstanced(drawIndexCount_, 1, 0, 0, 0);
}
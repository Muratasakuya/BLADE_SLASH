#include "CanvasCommon.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	CanvasCommon classMethods
//============================================================================

void BaseCanvas::AddRenderResource(BufferType type, uint32_t rootParamIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferAddress) {

	// 同じルートパラメータは上書き不可
	for (const auto& resource : renderResources_) {
		if (resource.rootParamIndex_ == rootParamIndex) {
			return;
		}
	}

	// 描画用リソース追加
	RenderResource resource;
	resource.type_ = type;
	resource.rootParamIndex_ = rootParamIndex;
	resource.bufferAddress_ = bufferAddress;
	renderResources_.emplace_back(resource);
}

void BaseCanvas::SetRenderResources(ID3D12GraphicsCommandList6* commandList) {

	// 描画用リソース設定
	for (const auto& resource : renderResources_) {
		// タイプ別にコマンドを分ける
		switch (resource.type_) {
		case BufferType::Constant:

			commandList->SetGraphicsRootConstantBufferView(resource.rootParamIndex_, resource.bufferAddress_);
			break;
		case BufferType::Structured:

			commandList->SetGraphicsRootShaderResourceView(resource.rootParamIndex_, resource.bufferAddress_);
			break;
		}
	}
}

void BaseCanvas::ImGuiCommon(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	ImGui::Checkbox("postProcessEnable", &postProcessEnable_);
	EnumAdapter<CanvasLayer>::Combo("CanvasLayer", &layer_);

	ImGui::Separator();

	// 現在のlayerIndex_から基準カテゴリを復元
	CanvasLayerIndex base = CanvasLayerIndex::None;
	uint16_t baseVal = 0;
	for (uint32_t i = 0; i < EnumAdapter<CanvasLayerIndex>::GetEnumCount(); ++i) {

		CanvasLayerIndex v = EnumAdapter<CanvasLayerIndex>::GetValue(i);
		uint16_t vv = static_cast<uint16_t>(v);
		if (vv <= layerIndex_) {
			base = v;
			baseVal = vv;
		}
	}

	// 次のカテゴリ境界(>baseVal で最小の値)を探索
	uint16_t nextBoundary = (std::numeric_limits<uint16_t>::max)();
	for (uint32_t i = 0; i < EnumAdapter<CanvasLayerIndex>::GetEnumCount(); ++i) {
		uint16_t vv = static_cast<uint16_t>(EnumAdapter<CanvasLayerIndex>::GetValue(i));
		if (vv > baseVal && vv < nextBoundary) {
			nextBoundary = vv;
		}
	}
	uint16_t maxSub = (nextBoundary == (std::numeric_limits<uint16_t>::max)()) ?
		(std::numeric_limits<uint16_t>::max)() - baseVal :
		static_cast<uint16_t>(nextBoundary - baseVal - 1);

	// カテゴリ選択 + カテゴリ内順序
	bool changed = false;
	changed |= EnumAdapter<CanvasLayerIndex>::Combo("Layer Index", &base);

	// カテゴリ変更されたら基準値を更新して上限も再計算
	baseVal = static_cast<uint16_t>(base);
	nextBoundary = (std::numeric_limits<uint16_t>::max)();
	for (uint32_t i = 0; i < EnumAdapter<CanvasLayerIndex>::GetEnumCount(); ++i) {

		uint16_t vv = static_cast<uint16_t>(EnumAdapter<CanvasLayerIndex>::GetValue(i));
		if (vv > baseVal && vv < nextBoundary) {
			nextBoundary = vv;
		}
	}
	maxSub = (nextBoundary == (std::numeric_limits<uint16_t>::max)()) ?
		(std::numeric_limits<uint16_t>::max)() - baseVal :
		static_cast<uint16_t>(nextBoundary - baseVal - 1);

	int subInt = static_cast<int>(layerIndex_ - baseVal);
	changed |= ImGui::DragInt("Order Category", &subInt, 1.0f, 0, static_cast<int>(maxSub));
	if (changed) {
		if (subInt < 0) {

			subInt = 0;
		}
		if (subInt > static_cast<int>(maxSub)) {

			subInt = static_cast<int>(maxSub);
		}
		layerIndex_ = static_cast<uint16_t>(baseVal + static_cast<uint16_t>(subInt));
	}
	ImGui::SameLine();
	ImGui::TextDisabled("(abs: %u)", static_cast<unsigned>(layerIndex_));

	ImGui::Separator();

	EnumAdapter<BlendMode>::Combo("BlendMode", &blendMode_);

	ImGui::PopItemWidth();
}
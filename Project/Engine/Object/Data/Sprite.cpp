#include "Sprite.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

// imgui
#include <imgui.h>

//============================================================================
//	Sprite classMethods
//============================================================================

namespace {

	// 頂点
	uint32_t LeftBottom = static_cast<uint32_t>(SpriteVertexPos::LeftBottom);
	uint32_t LeftTop = static_cast<uint32_t>(SpriteVertexPos::LeftTop);
	uint32_t RightBottom = static_cast<uint32_t>(SpriteVertexPos::RightBottom);
	uint32_t RightTop = static_cast<uint32_t>(SpriteVertexPos::RightTop);
}

Sprite::Sprite(ID3D12Device* device, Asset* asset,
	const std::string& textureName, Transform2D& transform) {

	asset_ = nullptr;
	asset_ = asset;

	textureName_ = textureName;
	preTextureName_ = textureName;
	metadata_ = asset_->GetMetaData(textureName_);

	layer_ = SpriteLayer::PostModel;

	// buffer作成
	InitBuffer(device);

	// textureSizeにtransformを合わせる
	SetMetaDataTextureSize(transform);
}

void Sprite::UpdateVertex(const Transform2D& transform) {

	float left = (0.0f - transform.anchorPoint.x) * transform.size.x;
	float right = (1.0f - transform.anchorPoint.x) * transform.size.x;
	float top = (0.0f - transform.anchorPoint.y) * transform.size.y;
	float bottom = (1.0f - transform.anchorPoint.y) * transform.size.y;

	// textureに変更があったときのみ
	if (preTextureName_ != textureName_) {

		// metaData更新
		metadata_ = asset_->GetMetaData(textureName_);
		preTextureName_ = textureName_;
	}

	// 横
	float texLeft = transform.textureLeftTop.x / static_cast<float>(metadata_.width);
	float texRight = (transform.textureLeftTop.x + transform.textureSize.x) / static_cast<float>(metadata_.width);
	// 縦
	float texTop = transform.textureLeftTop.y / static_cast<float>(metadata_.height);
	float texBottom = (transform.textureLeftTop.y + transform.textureSize.y) / static_cast<float>(metadata_.height);

	// vertexデータの更新
	// 左下
	vertexData_[LeftBottom].pos = Vector2(left, bottom) + transform.vertexOffset_[LeftBottom];
	vertexData_[LeftBottom].texcoord = { texLeft,texBottom };
	// 左上
	vertexData_[LeftTop].pos = Vector2(left, top) + transform.vertexOffset_[1];
	vertexData_[LeftTop].texcoord = { texLeft,texTop };
	// 右下
	vertexData_[RightBottom].pos = Vector2(right, bottom) + transform.vertexOffset_[2];
	vertexData_[RightBottom].texcoord = { texRight,texBottom };
	// 右上
	vertexData_[RightTop].pos = Vector2(right, top) + transform.vertexOffset_[3];
	vertexData_[RightTop].texcoord = { texRight,texTop };

	// GPUデータ転送
	vertexBuffer_.TransferData(vertexData_);
}

void Sprite::InitBuffer(ID3D12Device* device) {

	// buffer作成
	vertexBuffer_.CreateBuffer(device, kVertexNum_);
	indexBuffer_.CreateBuffer(device, kIndexNum_);

	// vertexデータの初期化
	vertexData_.resize(kVertexNum_);

	// indexデータの初期化
	std::vector<uint32_t> indexData(kIndexNum_);

	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	// GPUデータ転送
	indexBuffer_.TransferData(indexData);
}

void Sprite::SetMetaDataTextureSize(Transform2D& transform) {

	// textureMetadataの取得
	const DirectX::TexMetadata& metadata = asset_->GetMetaData(textureName_);

	// textureSizeの設定
	transform.textureSize = { static_cast<float>(metadata.width) ,static_cast<float>(metadata.height) };
	transform.size = transform.textureSize;

	// アンカー基準のピクセル矩形
	float left = (0.0f - transform.anchorPoint.x) * transform.size.x;
	float right = (1.0f - transform.anchorPoint.x) * transform.size.x;
	float top = (0.0f - transform.anchorPoint.y) * transform.size.y;
	float bottom = (1.0f - transform.anchorPoint.y) * transform.size.y;

	// 横
	float texLeft = transform.textureLeftTop.x / static_cast<float>(metadata.width);
	float texRight = (transform.textureLeftTop.x + transform.textureSize.x) / static_cast<float>(metadata.width);
	// 縦
	float texTop = transform.textureLeftTop.y / static_cast<float>(metadata.height);
	float texBottom = (transform.textureLeftTop.y + transform.textureSize.y) / static_cast<float>(metadata.height);

	// vertexデータの初期化
	// 左下
	vertexData_[LeftBottom].pos = { left,bottom };
	vertexData_[LeftBottom].texcoord = { texLeft,texBottom };
	vertexData_[LeftBottom].color = SakuEngine::Color::White();
	// 左上
	vertexData_[LeftTop].pos = { left,top };
	vertexData_[LeftTop].texcoord = { texLeft,texTop };
	vertexData_[LeftTop].color = SakuEngine::Color::White();
	// 右下
	vertexData_[RightBottom].pos = { right,bottom };
	vertexData_[RightBottom].texcoord = { texRight,texBottom };
	vertexData_[RightBottom].color = SakuEngine::Color::White();
	// 右上
	vertexData_[RightTop].pos = { right,top };
	vertexData_[RightTop].texcoord = { texRight,texTop };
	vertexData_[RightTop].color = SakuEngine::Color::White();
}


void Sprite::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	ImGui::SeparatorText("Vertex Color");

	// 頂点カラー編集
	// 左下
	ImGui::ColorEdit4("leftBottomColor", &vertexData_[LeftBottom].color.r);
	// 左上
	ImGui::ColorEdit4("leftTopColor", &vertexData_[LeftTop].color.r);
	// 右下
	ImGui::ColorEdit4("rightBottomColor", &vertexData_[RightBottom].color.r);
	// 右上
	ImGui::ColorEdit4("rightTopColor", &vertexData_[RightTop].color.r);

	ImGui::Separator();

	// テクスチャ選択
	// 表示サイズ
	const float imageSize = 88.0f;
	SakuEngine::ImGuiHelper::ImageButtonWithLabel("texture", textureName_,
		(ImTextureID)asset_->GetGPUHandle(textureName_).ptr, { imageSize, imageSize });
	std::string dragTextureName = SakuEngine::ImGuiHelper::DragDropPayloadString(PendingType::Texture);
	if (!dragTextureName.empty()) {

		// textureを設定
		textureName_ = dragTextureName;
	}

	ImGui::Checkbox("postProccessEnable", &postProccessEnable_);
	SakuEngine::EnumAdapter<SpriteLayer>::Combo("SpriteLayer", &layer_);
	ImGui::Separator();

	// 現在のlayerIndex_から「基準カテゴリ(base)」を復元
	SpriteLayerIndex base = SpriteLayerIndex::None;
	uint16_t baseVal = 0;
	for (uint32_t i = 0; i < SakuEngine::EnumAdapter<SpriteLayerIndex>::GetEnumCount(); ++i) {

		SpriteLayerIndex v = SakuEngine::EnumAdapter<SpriteLayerIndex>::GetValue(i);
		uint16_t vv = static_cast<uint16_t>(v);
		if (vv <= layerIndex_) {
			base = v;
			baseVal = vv;
		}
	}

	// 次のカテゴリ境界(>baseVal で最小の値)を探索
	uint16_t nextBoundary = (std::numeric_limits<uint16_t>::max)();
	for (uint32_t i = 0; i < SakuEngine::EnumAdapter<SpriteLayerIndex>::GetEnumCount(); ++i) {
		uint16_t vv = static_cast<uint16_t>(SakuEngine::EnumAdapter<SpriteLayerIndex>::GetValue(i));
		if (vv > baseVal && vv < nextBoundary) {
			nextBoundary = vv;
		}
	}
	uint16_t maxSub = (nextBoundary == (std::numeric_limits<uint16_t>::max)()) ?
		(std::numeric_limits<uint16_t>::max)() - baseVal :
		static_cast<uint16_t>(nextBoundary - baseVal - 1);

	// カテゴリ選択 + カテゴリ内順序
	bool changed = false;
	changed |= SakuEngine::EnumAdapter<SpriteLayerIndex>::Combo("Layer Index", &base);

	// カテゴリ変更されたら基準値を更新して上限も再計算
	baseVal = static_cast<uint16_t>(base);
	nextBoundary = (std::numeric_limits<uint16_t>::max)();
	for (uint32_t i = 0; i < SakuEngine::EnumAdapter<SpriteLayerIndex>::GetEnumCount(); ++i) {

		uint16_t vv = static_cast<uint16_t>(SakuEngine::EnumAdapter<SpriteLayerIndex>::GetValue(i));
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

	SakuEngine::EnumAdapter<BlendMode>::Combo("BlendMode", &blendMode_);

	ImGui::PopItemWidth();
}

void Sprite::ToJson(Json& data) {

	data["textureName"] = textureName_;
	data["postProccessEnable_"] = postProccessEnable_;
	data["layer"] = SakuEngine::EnumAdapter<SpriteLayer>::ToString(layer_);
	data["layerIndex"] = layerIndex_;
	data["blendMode"] = SakuEngine::EnumAdapter<BlendMode>::ToString(blendMode_);

	data["leftBottomColor"] = vertexData_[LeftBottom].color.ToJson();
	data["leftTopColor"] = vertexData_[LeftTop].color.ToJson();
	data["rightBottomColor"] = vertexData_[RightBottom].color.ToJson();
	data["rightTopColor"] = vertexData_[RightTop].color.ToJson();
}

void Sprite::FromJson(const Json& data) {

	postProccessEnable_ = data.value("postProccessEnable_", false);
	textureName_ = data["textureName"].get<std::string>();
	layer_ = SakuEngine::EnumAdapter<SpriteLayer>::FromString(data["layer"].get<std::string>()).value();
	layerIndex_ = data["layerIndex"].get<uint16_t>();
	blendMode_ = SakuEngine::EnumAdapter<BlendMode>::FromString(data["blendMode"].get<std::string>()).value();

	if (data.contains("leftBottomColor")) {

		vertexData_[LeftBottom].color = SakuEngine::Color::FromJson(data.value("leftBottomColor", Json()));
		vertexData_[LeftTop].color = SakuEngine::Color::FromJson(data.value("leftTopColor", Json()));
		vertexData_[RightBottom].color = SakuEngine::Color::FromJson(data.value("rightBottomColor", Json()));
		vertexData_[RightTop].color = SakuEngine::Color::FromJson(data.value("rightTopColor", Json()));
	}
}

const D3D12_GPU_DESCRIPTOR_HANDLE& Sprite::GetTextureGPUHandle() const {

	return asset_->GetGPUHandle(textureName_);
}

const D3D12_GPU_DESCRIPTOR_HANDLE& Sprite::GetAlphaTextureGPUHandle() const {

	if (!alphaTextureName_.has_value()) {
		assert(false && "alpha texture name is not set");
	}

	return asset_->GetGPUHandle(alphaTextureName_.value());
}

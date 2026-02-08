#include "Sprite.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Input/Input.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/Data/Material/Material.h>
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
	for (auto& name : deviceTextureNames_) {
		name = textureName;
	}

	// buffer作成
	InitBuffer(device);

	// textureSizeにtransformを合わせる
	SetMetaDataTextureSize(transform);
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

void Sprite::UpdateVertex(const Transform2D& transform, const SpriteMaterial& material) {

	// デバイスに応じたテクスチャ名更新
	UpdateDeviceTextureName();

	float left = (0.0f - transform.anchorPoint.x) * transform.size.x;
	float right = (1.0f - transform.anchorPoint.x) * transform.size.x;
	float top = (0.0f - transform.anchorPoint.y) * transform.size.y;
	float bottom = (1.0f - transform.anchorPoint.y) * transform.size.y;

	// textureに変更があったときのみ
	if (preTextureName_ != textureName_) {

		// metaData更新
		metadata_ = asset_->GetMetaData(textureName_);
	}

	// 横
	float texLeft = transform.textureLeftTop.x / static_cast<float>(metadata_.width);
	float texRight = (transform.textureLeftTop.x + transform.textureSize.x) / static_cast<float>(metadata_.width);
	// 縦
	float texTop = transform.textureLeftTop.y / static_cast<float>(metadata_.height);
	float texBottom = (transform.textureLeftTop.y + transform.textureSize.y) / static_cast<float>(metadata_.height);

	// vertexデータの更新
	// 左下
	vertexData_[LeftBottom].pos = Vector2(left, bottom) + transform.vertexOffset[LeftBottom];
	vertexData_[LeftBottom].texcoord = { texLeft,texBottom };
	// 左上
	vertexData_[LeftTop].pos = Vector2(left, top) + transform.vertexOffset[1];
	vertexData_[LeftTop].texcoord = { texLeft,texTop };
	// 右下
	vertexData_[RightBottom].pos = Vector2(right, bottom) + transform.vertexOffset[2];
	vertexData_[RightBottom].texcoord = { texRight,texBottom };
	// 右上
	vertexData_[RightTop].pos = Vector2(right, top) + transform.vertexOffset[3];
	vertexData_[RightTop].texcoord = { texRight,texTop };

	// GPUデータ転送
	vertexBuffer_.TransferData(vertexData_);

	// 描画するかどうかを更新する
	isDrawEnable_ |= 0.0f < transform.GetWorldScale().Length();
	isDrawEnable_ |= 0.0f < material.material.color.a;
}

void Sprite::SetRenderResources(uint32_t objectId) {

	objectId_ = objectId;

	// 必要なデータを取得
	ObjectManager* objectManager = ObjectManager::GetInstance();
	auto* transform = objectManager->GetData<Transform2D>(objectId);
	auto* material = objectManager->GetData<SpriteMaterial>(objectId);

	// リソース追加
	AddRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::Constant,
		.rootParamIndex = 1,
		.bufferAddress = transform->GetBuffer().GetResource()->GetGPUVirtualAddress(),
		.bufferHandle = D3D12_GPU_DESCRIPTOR_HANDLE{},
		});
	AddRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::TextureGPU,
		.rootParamIndex = 2,
		.bufferAddress = 0,
		.bufferHandle = GetTextureGPUHandle(),
		});
	AddRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::TextureGPU,
		.rootParamIndex = 3,
		.bufferAddress = 0,
		.bufferHandle = GetAlphaTextureGPUHandle(),
		});
	AddRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::Constant,
		.rootParamIndex = 4,
		.bufferAddress = material->GetBuffer().GetResource()->GetGPUVirtualAddress(),
		.bufferHandle = D3D12_GPU_DESCRIPTOR_HANDLE{},
		});
}

void Sprite::DrawCommand(ID3D12GraphicsCommandList6* commandList) {

	// テクスチャの名前に変更があった場合、再度リソース設定を行う
	if (preTextureName_ != textureName_) {

		// 描画リソース再設定
		OverWriteRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::TextureGPU,
		.rootParamIndex = 2,
		.bufferAddress = 0,
		.bufferHandle = GetTextureGPUHandle(),
			});
		preTextureName_ = textureName_;
	}
	if (preAlphaTextureName_ != alphaTextureName_) {

		// 描画リソース再設定
		OverWriteRenderResource(RenderResource{
		.type = BaseCanvas::BufferType::TextureGPU,
		.rootParamIndex = 3,
		.bufferAddress = 0,
		.bufferHandle = GetAlphaTextureGPUHandle(),
			});
		preAlphaTextureName_ = alphaTextureName_;
	}

	// 描画有効でなければ処理しない
	if (!isDrawEnable_) {
		return;
	}

	// 頂点バッファ設定
	commandList->IASetVertexBuffers(0, 1, &vertexBuffer_.GetVertexBufferView());
	// インデックスバッファ設定
	commandList->IASetIndexBuffer(&indexBuffer_.GetIndexBufferView());
	// 描画リソース
	BaseCanvas::SetRenderResourceCommand(commandList);
	// 描画コマンド
	commandList->DrawIndexedInstanced(kIndexNum_, 1, 0, 0, 0);
}

void Sprite::UpdateDeviceTextureName() {

	// falseなら処理しない
	if (!isChangeDeviceTexture_) {
		return;
	}

	// 現在の入力デバイスを取得
	InputType inputType = Input::GetInstance()->GetType();
	// テクスチャ名を更新
	if (!deviceTextureNames_[static_cast<uint32_t>(inputType)].empty()) {

		textureName_ = deviceTextureNames_[static_cast<uint32_t>(inputType)];
	}
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
	vertexData_[LeftBottom].color = Color::White();
	// 左上
	vertexData_[LeftTop].pos = { left,top };
	vertexData_[LeftTop].texcoord = { texLeft,texTop };
	vertexData_[LeftTop].color = Color::White();
	// 右下
	vertexData_[RightBottom].pos = { right,bottom };
	vertexData_[RightBottom].texcoord = { texRight,texBottom };
	vertexData_[RightBottom].color = Color::White();
	// 右上
	vertexData_[RightTop].pos = { right,top };
	vertexData_[RightTop].texcoord = { texRight,texTop };
	vertexData_[RightTop].color = Color::White();
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

	ImGui::PopItemWidth();

	// テクスチャ選択
	// 表示サイズ
	const float imageSize = 88.0f;
	ImGui::PushID("MainTexture");
	ImGuiHelper::ImageButtonWithLabel("texture", textureName_,
		(ImTextureID)asset_->GetGPUHandle(textureName_).ptr, { imageSize, imageSize });
	std::string dragTextureName = ImGuiHelper::DragDropPayloadString(PendingType::Texture);
	if (!dragTextureName.empty()) {

		// textureを設定
		textureName_ = dragTextureName;
	}
	ImGui::PopID();

	if (ImGui::CollapsingHeader("Device Texture")) {

		ImGui::Checkbox("isChangeDeviceTexture", &isChangeDeviceTexture_);
		// 入力デバイスごとのテクスチャ設定
		{
			// キーボード
			ImGui::SeparatorText("Keyboard/Mouse Texture");
			ImGui::PushID("KeyboardTexture");

			auto& textureName = deviceTextureNames_[static_cast<uint32_t>(InputType::Keyboard)];
			ImGuiHelper::ImageButtonWithLabel("texture", textureName,
				(ImTextureID)asset_->GetGPUHandle(textureName).ptr, { imageSize, imageSize });
			dragTextureName = ImGuiHelper::DragDropPayloadString(PendingType::Texture);
			if (!dragTextureName.empty()) {

				// textureを設定
				textureName = dragTextureName;
			}
			ImGui::PopID();
		}
		{
			// ゲームパッド
			ImGui::SeparatorText("Gamepad Texture");
			ImGui::PushID("GamePadTexture");

			auto& textureName = deviceTextureNames_[static_cast<uint32_t>(InputType::GamePad)];
			ImGuiHelper::ImageButtonWithLabel("texture", textureName,
				(ImTextureID)asset_->GetGPUHandle(textureName).ptr, { imageSize, imageSize });
			dragTextureName = ImGuiHelper::DragDropPayloadString(PendingType::Texture);
			if (!dragTextureName.empty()) {

				// textureを設定
				textureName = dragTextureName;
			}
			ImGui::PopID();
		}
	}

	// 共通エディター
	BaseCanvas::ImGuiCommon(itemSize);
}

void Sprite::ToJson(Json& data) {

	data["textureName"] = textureName_;
	data["layer"] = EnumAdapter<CanvasLayer>::ToString(layer_);
	data["layerIndex"] = layerIndex_;
	data["blendMode"] = EnumAdapter<BlendMode>::ToString(blendMode_);

	data["leftBottomColor"] = vertexData_[LeftBottom].color.ToJson();
	data["leftTopColor"] = vertexData_[LeftTop].color.ToJson();
	data["rightBottomColor"] = vertexData_[RightBottom].color.ToJson();
	data["rightTopColor"] = vertexData_[RightTop].color.ToJson();

	for (uint32_t i = 0; i < deviceTextureNames_.size(); ++i) {

		data["deviceTextureNames"][i] = deviceTextureNames_[i];
	}
	data["isChangeDeviceTexture"] = isChangeDeviceTexture_;

	// 1度更新する
	UpdateDeviceTextureName();
}

void Sprite::FromJson(const Json& data) {

	textureName_ = data["textureName"].get<std::string>();
	layer_ = EnumAdapter<CanvasLayer>::FromString(data["layer"].get<std::string>()).value();
	layerIndex_ = data["layerIndex"].get<uint16_t>();
	blendMode_ = EnumAdapter<BlendMode>::FromString(data["blendMode"].get<std::string>()).value();

	if (data.contains("leftBottomColor")) {

		vertexData_[LeftBottom].color = Color::FromJson(data.value("leftBottomColor", Json()));
		vertexData_[LeftTop].color = Color::FromJson(data.value("leftTopColor", Json()));
		vertexData_[RightBottom].color = Color::FromJson(data.value("rightBottomColor", Json()));
		vertexData_[RightTop].color = Color::FromJson(data.value("rightTopColor", Json()));
	}

	if (data.contains("deviceTextureNames")) {

		auto deviceTextureNamesJson = data["deviceTextureNames"];
		for (uint32_t i = 0; i < deviceTextureNamesJson.size() && i < deviceTextureNames_.size(); ++i) {

			deviceTextureNames_[i] = deviceTextureNamesJson[i].get<std::string>();
		}
	}
	isChangeDeviceTexture_ = data.value("isChangeDeviceTexture", false);
}

const D3D12_GPU_DESCRIPTOR_HANDLE& Sprite::GetTextureGPUHandle() const {

	return asset_->GetGPUHandle(textureName_);
}

D3D12_GPU_DESCRIPTOR_HANDLE Sprite::GetAlphaTextureGPUHandle() const {

	if (!alphaTextureName_.has_value()) {
		// アルファテクスチャ未使用、空のハンドルを返す
		return D3D12_GPU_DESCRIPTOR_HANDLE{};
	}

	return asset_->GetGPUHandle(alphaTextureName_.value());
}

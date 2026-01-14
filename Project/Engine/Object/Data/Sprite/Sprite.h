#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetStructure.h>
#include <Engine/Object/Data/Transform/Transform.h>
#include <Engine/Core/Graphics/GPUObject/VertexBuffer.h>
#include <Engine/Core/Graphics/GPUObject/IndexBuffer.h>
#include <Engine/Core/Graphics/DxLib/DxStructures.h>
#include <Engine/Input/InputStructures.h>
#include <Engine/Object/Data/Canvas/CanvasCommon.h>

// directX
#include <Externals/DirectXTex/DirectXTex.h>
// c++
#include <string>

namespace SakuEngine {

	// front
	class Asset;

	//============================================================================
	//	enum class
	//============================================================================

	// スプライト頂点位置
	enum class SpriteVertexPos {

		LeftBottom,
		LeftTop,
		RightBottom,
		RightTop
	};
	// 頂点数
	static constexpr const uint32_t kSpriteVertexPosNum = 4;

	//============================================================================
	//	Sprite class
	//	2Dスプライトデータ
	//============================================================================
	class Sprite :
		public BaseCanvas {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		Sprite() = default;
		// スプライトの初期化
		Sprite(ID3D12Device* device, Asset* asset,
			const std::string& textureName, Transform2D& transform);
		~Sprite() = default;

		// 頂点情報更新
		void UpdateVertex(const Transform2D& transform);

		// 描画リソースの設定
		void SetRenderResources(uint32_t objectId);
		// 描画コマンド
		void DrawCommand(ID3D12GraphicsCommandList6* commandList) override;

		// エディター
		void ImGui(float itemSize);

		// json
		void ToJson(Json& data);
		void FromJson(const Json& data);

		//--------- accessor -----------------------------------------------------

		// メタデータからテクスチャサイズ設定
		void SetMetaDataTextureSize(Transform2D& transform);

		// テクスチャ名の設定
		void SetTextureName(const std::string& textureName) { textureName_ = textureName; }
		void SetAlphaTextureName(const std::string& textureName) { alphaTextureName_ = textureName; }

		// 頂点カラーの設定
		void SetVertexColor(SpriteVertexPos pos, const Color& color) { vertexData_[static_cast<uint32_t>(pos)].color = color; }

		static uint32_t GetIndexNum() { return kIndexNum_; }
		uint16_t GetLayerIndex() const { return static_cast<uint16_t>(layerIndex_); }
		bool UseAlphaTexture() const { return alphaTextureName_.has_value(); }
		const Color& GetVertexColor(SpriteVertexPos pos) const { return vertexData_[static_cast<uint32_t>(pos)].color; }

		// バッファ取得
		const VertexBuffer<SpriteVertexData>& GetVertexBuffer() const { return vertexBuffer_; }
		const IndexBuffer& GetIndexBuffer() const { return indexBuffer_; }
		// テクスチャGPUハンドル取得
		const D3D12_GPU_DESCRIPTOR_HANDLE& GetTextureGPUHandle() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetAlphaTextureGPUHandle() const;

		// 描画タイプ取得
		CanvasType GetType() const override { return CanvasType::Sprite; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		static constexpr const uint32_t kVertexNum_ = 4;
		static constexpr const uint32_t kIndexNum_ = 6;

		Asset* asset_;

		// テクスチャ関連
		std::string textureName_;
		std::string preTextureName_;
		std::optional<std::string> alphaTextureName_;
		std::optional<std::string> preAlphaTextureName_;

		DirectX::TexMetadata metadata_;
		// 現在の入力デバイスに応じてテクスチャを変更するか
		bool isChangeDeviceTexture_ = false;
		std::array<std::string, 2> deviceTextureNames_{};

		// 頂点情報
		std::vector<SpriteVertexData> vertexData_;

		// buffer
		VertexBuffer<SpriteVertexData> vertexBuffer_;
		IndexBuffer indexBuffer_;

		//--------- functions ----------------------------------------------------

		// バッファ作成
		void InitBuffer(ID3D12Device* device);

		// 入力タイプをチェックしてテクスチャ名を更新
		void UpdateDeviceTextureName();
	};
}; // SakuEngine
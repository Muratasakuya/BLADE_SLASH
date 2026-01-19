#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxStructures.h>

// c++
#include <cstdint>

//============================================================================
//	CanvasCommon class
//============================================================================

namespace SakuEngine {

	// タイプ
	enum class CanvasType {

		Sprite,
		Text,
		Count
	};
	// レイヤー
	enum class CanvasLayer {

		PreModel, // モデルの前に描画する
		PostModel // モデルの後に描画する
	};
	// レイヤーインデックス
	enum class CanvasLayerIndex :
		uint16_t {

		None = 0,              // 一番手前の表示(初期化順で決まる)
		SceneTransition = 128, // シーン遷移処理
	};

	//============================================================================
	//	BaseCanvas class
	//	キャンバス共通基底クラス
	//============================================================================
	class BaseCanvas {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// バッファ種別
		enum class BufferType {

			Constant,   // 定数バッファ
			Structured, // 構造化バッファ
			TextureGPU,
		};

		// 頂点以外の描画データ
		struct RenderResource {

			// バッファ種別
			BufferType type;

			// 描画コマンド情報
			uint32_t rootParamIndex;
			D3D12_GPU_VIRTUAL_ADDRESS bufferAddress;
			D3D12_GPU_DESCRIPTOR_HANDLE bufferHandle;
		};
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		BaseCanvas() = default;
		virtual ~BaseCanvas() = default;

		// 共通エディター
		void ImGuiCommon(float itemSize);

		// 描画コマンド
		virtual void DrawCommand(ID3D12GraphicsCommandList6* commandList) = 0;

		//--------- accessor -----------------------------------------------------

		// 描画情報セット
		void SetLayer(CanvasLayer layer) { layer_ = layer; }
		void SetLayerIndex(CanvasLayerIndex layerIndex, uint16_t subLayerIndex) { layerIndex_ = static_cast<uint16_t>(layerIndex) + subLayerIndex; }
		void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; }

		// レイヤー取得
		CanvasLayer GetLayer() const { return layer_; }
		// レイヤーインデックス取得
		uint16_t GetLayerIndex() const { return static_cast<uint16_t>(layerIndex_); }
		// 描画モード取得
		BlendMode GetBlendMode() const { return blendMode_; }

		// タイプ取得
		virtual CanvasType GetType() const = 0;
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// オブジェクトID
		uint32_t objectId_ = 0;

		// 描画順制御
		CanvasLayer layer_ = CanvasLayer::PostModel;
		uint16_t layerIndex_ = static_cast<uint16_t>(CanvasLayerIndex::None);

		// 描画モード
		BlendMode blendMode_ = BlendMode::kBlendModeNormal;

		//--------- functions ----------------------------------------------------

		// 描画用リソース追加
		void AddRenderResource(const RenderResource& renderResource);
		// 描画用リソースの再設定
		void OverWriteRenderResource(const RenderResource& renderResource);

		// 描画用リソースコマンド設定
		void SetRenderResourceCommand(ID3D12GraphicsCommandList6* commandList);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 描画用リソース
		std::vector<RenderResource> renderResources_;
	};
}
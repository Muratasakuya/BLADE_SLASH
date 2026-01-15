#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>

// c++
#include <optional>

namespace SakuEngine {

	// 矩形
	struct MSDFPlaneBounds {

		float left = 0.0f;
		float bottom = 0.0f;
		float right = 0.0f;
		float top = 0.0f;
	};
	struct MSDFAtlasBounds {

		int32_t left = 0;
		int32_t bottom = 0;
		int32_t right = 0;
		int32_t top = 0;
	};

	// グリフ情報
	struct MSDFGlyph {

		char32_t codepoint = U'?'; // 文字
		float advance = 0.0f;

		std::optional<MSDFPlaneBounds> planeBounds;
		std::optional<MSDFAtlasBounds> atlasBounds;
	};

	// フォント全体のメトリクス
	struct MSDFMetrics {

		float emSize = 0.0f;
		float lineHeight = 0.0f;
		float ascender = 0.0f;
		float descender = 0.0f;
	};

	//============================================================================
	//	MSDFFont class
	//	MSDFフォントデータ
	//============================================================================
	class MSDFFont {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		MSDFFont() = default;
		~MSDFFont() = default;

		// 読み込みして初期化
		void Init(Asset* asset, const std::string& atlasTextureName, const std::string& jsonPath);

		// .jsonファイルを解析できるかどうか
		bool CanParseJson(const std::string& jsonPath) const;

		//--------- accessor -----------------------------------------------------

		// グリフ検索して返す
		const MSDFGlyph* FindGlyph(char32_t codepoint) const;
		// カーニング取得
		float GetKerning(char32_t left, char32_t right) const;

		// MSDFの距離レンジ
		float GetPxRange() const { return pxRange_; }
		// フォント全体のメトリクス
		const MSDFMetrics& GetMetrics() const { return metrics_; }

		// アトラスサイズ
		uint32_t GetAtlasWidth() const { return atlasWidth_; }
		uint32_t GetAtlasHeight() const { return atlasHeight_; }
		Vector2 GetAtlasSize() const { return Vector2(static_cast<float>(atlasWidth_), static_cast<float>(atlasHeight_)); }

		// テクスチャ名の取得
		const std::string& GetAtlasTextureName() const { return atlasTextureName_; }
		// アトラステクスチャのGPUハンドル
		const D3D12_GPU_DESCRIPTOR_HANDLE& GetAtlasGPUHandle() const { return asset_->GetGPUHandle(atlasTextureName_); }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		Asset* asset_;

		// アトラステクスチャ名
		std::string atlasTextureName_;

		// MSDFの距離レンジ
		float pxRange_ = 0.0f;
		// アトラスサイズ
		uint32_t atlasWidth_ = 0;
		uint32_t atlasHeight_ = 0;
		float fontPixelsPerEm_ = 0.0f;

		// フォント全体のメトリクス
		MSDFMetrics metrics_{};

		// グリフマップ
		std::unordered_map<char32_t, MSDFGlyph> glyphMap_;
		std::unordered_map<uint64_t, float> kerningMap_;

		//--------- functions ----------------------------------------------------

		// ファイル全体を文字列として読む
		static std::string ReadAllText(const std::string& path);
	};
}; // SakuEngine
#include "MSDFFont.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>

//============================================================================
//	MSDFFont classMethods
//============================================================================

namespace {

	// カーニングキー作成
	uint64_t MakeKerningKey(char32_t left, char32_t right) {
		return (uint64_t(uint32_t(left)) << 32) | uint64_t(uint32_t(right));
	}
}

void MSDFFont::Init(Asset* asset, const std::string& atlasTextureName, const std::string& jsonPath) {

	asset_ = asset;
	atlasTextureName_ = atlasTextureName;

	// json読み込み
	const std::string text = ReadAllText(jsonPath);
	const nlohmann::json jsonData = nlohmann::json::parse(text);

	//============================================================================
	// アトラスの解析
	//============================================================================

	if (jsonData.contains("atlas")) {

		const auto& atlas = jsonData["atlas"];
		if (atlas.contains("distanceRange")) {

			pxRange_ = atlas["distanceRange"].get<float>();
		} else if (atlas.contains("pxRange")) {

			pxRange_ = atlas["pxRange"].get<float>();
		}
		if (atlas.contains("size")) {

			atlasWidth_ = atlas["size"][0].get<uint32_t>();
			atlasHeight_ = atlas["size"][1].get<uint32_t>();
		} else {

			// sizeが無い場合はAssetのmetadataを使う
			const auto& md = asset_->GetMetaData(atlasTextureName_);
			atlasWidth_ = uint32_t(md.width);
			atlasHeight_ = uint32_t(md.height);
		}
	} else {

		// atlas情報が無い場合もAssetのmetadataに逃がす
		const auto& metaData = asset_->GetMetaData(atlasTextureName_);
		atlasWidth_ = uint32_t(metaData.width);
		atlasHeight_ = uint32_t(metaData.height);

		// デフォルト値
		pxRange_ = 8.0f;
	}

	//============================================================================
	// メトリクスの解析
	//============================================================================

	if (jsonData.contains("metrics")) {

		const auto& metrics = jsonData["metrics"];

		metrics_.emSize = metrics.value("emSize", 0.0f);
		metrics_.lineHeight = metrics.value("lineHeight", 0.0f);
		metrics_.ascender = metrics.value("ascender", 0.0f);
		metrics_.descender = metrics.value("descender", 0.0f);
	}

	//============================================================================
	// グリフの解析
	//============================================================================

	glyphMap_.clear();
	if (jsonData.contains("glyphs")) {
		for (const auto& glyphJson : jsonData["glyphs"]) {

			MSDFGlyph glyph;

			if (glyphJson.contains("unicode")) {

				glyph.codepoint = char32_t(glyphJson["unicode"].get<uint32_t>());
			} else if (glyphJson.contains("codepoint")) {

				glyph.codepoint = char32_t(glyphJson["codepoint"].get<uint32_t>());
			} else {
				continue;
			}

			glyph.advance = glyphJson.value("advance", 0.0f);

			if (glyphJson.contains("planeBounds") && !glyphJson["planeBounds"].is_null()) {

				MSDFPlaneBounds planeBounds;

				const auto& planeBoundsJson = glyphJson["planeBounds"];
				planeBounds.left = planeBoundsJson["left"].get<float>();
				planeBounds.bottom = planeBoundsJson["bottom"].get<float>();
				planeBounds.right = planeBoundsJson["right"].get<float>();
				planeBounds.top = planeBoundsJson["top"].get<float>();
				glyph.planeBounds = planeBounds;
			}
			if (glyphJson.contains("atlasBounds") && !glyphJson["atlasBounds"].is_null()) {

				MSDFAtlasBounds atlasBounds;

				const auto& atlasBoundsJson = glyphJson["atlasBounds"];
				atlasBounds.left = atlasBoundsJson["left"].get<int>();
				atlasBounds.bottom = atlasBoundsJson["bottom"].get<int>();
				atlasBounds.right = atlasBoundsJson["right"].get<int>();
				atlasBounds.top = atlasBoundsJson["top"].get<int>();
				glyph.atlasBounds = atlasBounds;
			}
			glyphMap_[glyph.codepoint] = glyph;
		}
	}

	//============================================================================
	// カーニングの解析
	//============================================================================

	kerningMap_.clear();
	if (jsonData.contains("kerning")) {
		for (const auto& kerningJson : jsonData["kerning"]) {

			char32_t u1 = char32_t(kerningJson["unicode1"].get<uint32_t>());
			char32_t u2 = char32_t(kerningJson["unicode2"].get<uint32_t>());
			float adv = kerningJson["advance"].get<float>();
			kerningMap_[MakeKerningKey(u1, u2)] = adv;
		}
	}

	// 最低限の値チェック
	assert(atlasWidth_ > 0 && atlasHeight_ > 0);
	assert(pxRange_ > 0.0f);
}

std::string MSDFFont::ReadAllText(const std::string& path) {

	std::ifstream ifs(path, std::ios::binary);
	// ファイルを開けなかった場合エラー
	assert(ifs && "MSDFFont json file open failed");

	std::ostringstream oss;
	oss << ifs.rdbuf();
	return oss.str();
}

const MSDFGlyph* MSDFFont::FindGlyph(char32_t codepoint) const {

	auto it = glyphMap_.find(codepoint);
	if (it != glyphMap_.end()) {
		return &it->second;
	}
	auto it2 = glyphMap_.find(U'?');
	if (it2 != glyphMap_.end()) {
		return &it2->second;
	}
	return nullptr;
}

float MSDFFont::GetKerning(char32_t left, char32_t right) const {

	auto it = kerningMap_.find(MakeKerningKey(left, right));
	if (it != kerningMap_.end()) {
		return it->second;
	}
	return 0.0f;
}
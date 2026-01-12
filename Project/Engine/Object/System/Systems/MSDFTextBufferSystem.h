#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/Text/MSDFText.h>

namespace SakuEngine {

	// front
	class TextTransform2D;
	class MSDFTextMaterial;

	//============================================================================
	//	structure
	//============================================================================

	struct MSDFTextData {

		TextTransform2D* transform;
		MSDFTextMaterial* material;
		MSDFText* text;
	};

	//============================================================================
	//	MSDFTextBufferSystem class
	//	MSDFテキストのバッファを管理するシステム
	//============================================================================
	class MSDFTextBufferSystem :
		public ISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		MSDFTextBufferSystem() = default;
		~MSDFTextBufferSystem() = default;

		// MSDFフォントを作成、キャッシュがあればそれを返す
		const MSDFFont* GetMSDFFont(class Asset* asset,
			const std::string& atlasTextureName, const std::string& fontJsonPath);

		Archetype Signature() const override;

		void Update(ObjectPoolManager& ObjectPoolManager) override;

		//--------- accessor -----------------------------------------------------

		const std::vector<MSDFTextData>& GetTextData() const { return textData_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		std::vector<MSDFTextData> textData_;

		// フォントキャッシュ
		std::unordered_map<std::string, std::unique_ptr<MSDFFont>> fontCache_;
	};
}; // SakuEngine
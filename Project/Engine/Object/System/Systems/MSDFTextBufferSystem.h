#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/Text/MSDFText.h>
#include <Engine/Editor/Base/EditorModuleRegistry.h>

namespace SakuEngine {

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

		MSDFTextBufferSystem();
		~MSDFTextBufferSystem() = default;

		// MSDFフォントを作成、キャッシュがあればそれを返す
		const MSDFFont* GetMSDFFont(class Asset* asset,
			const std::string& atlasTextureName, const std::string& fontJsonPath);

		Archetype Signature() const override;

		void Update(ObjectPoolManager& ObjectPoolManager) override;

		//--------- accessor -----------------------------------------------------

		// テキスト構築クラスの作成
		std::unique_ptr<ITextGenerator> CreateTextGenerator(TextGeneratorType type);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 登録されているクラスを管理するレジストリ
		using TextGeneratorRegistry = EditorModuleRegistry<ITextGenerator, TextGeneratorType>;

		// フォントキャッシュ
		std::unordered_map<std::string, std::unique_ptr<MSDFFont>> fontCache_;
	};
}; // SakuEngine
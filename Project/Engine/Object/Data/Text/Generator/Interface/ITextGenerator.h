#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/ImGuiHelper.h>

// c++
#include <string>

namespace SakuEngine {

	// テキスト生成タイプ
	enum class TextGeneratorType
		: uint32_t {

		Clock, // 時間表示
	};

	//============================================================================
	//	ITextGenerator class
	//	MSDFText に表示する UTF-8 文字列を生成するインターフェース
	//============================================================================
	class ITextGenerator {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		ITextGenerator() = default;
		virtual ~ITextGenerator() = default;

		// 文字列生成、変更があればtrueを返す
		virtual bool Generate(std::string& outString) = 0;

		// エディター
		virtual void ImGui() {}

		// json
		virtual void FromJson([[maybe_unused]] const Json& data) {}
		virtual void ToJson([[maybe_unused]] Json& data) const {}

		//--------- accessor -----------------------------------------------------

		// ジェネレータータイプ取得
		virtual TextGeneratorType GetType() const = 0;
	};
} // SakuEngine
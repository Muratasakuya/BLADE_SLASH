#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Text/Generator/Interface/ITextGenerator.h>

// c++
#include <cstdint>
#include <vector>
#include <functional>

namespace SakuEngine {

	//============================================================================
	//	ClockTextGenerator class
	//	値を時刻表示文字列に変換するテキストジェネレーター
	//============================================================================
	class ClockTextGenerator :
		public ITextGenerator {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// 表記パラメータ
		struct Params {

			// 表示パターン
			std::string pattern = "dd:dd.dd";

			// 負値になったら0表示にするか
			bool clampNegativeToZero = true;
			// clampしない場合に-記号を出すか
			bool showMinusSign = true;
			// 小数部を四捨五入するか
			bool roundFraction = false;
		};

		// パターン解析結果
		struct PatternInfo {

			std::vector<int32_t> groupWidths; // "dd"の個数リスト
			std::vector<std::string> seps;    // 区切り文字
			bool hasFraction = false;         // 小数部を持つか
		};
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		ClockTextGenerator() = default;
		~ClockTextGenerator() = default;

		bool Generate(std::string& outString) override;

		// エディター
		void ImGui() override;

		//--------- accessor -----------------------------------------------------

		// 秒数の設定
		void SetSeconds(float seconds);
		void SetSecondsGetter(std::function<float()> getter);

		// ジェネレータータイプ取得
		static constexpr TextGeneratorType ID = TextGeneratorType::Clock;
		TextGeneratorType GetType() const override { return ID; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// パラメータ
		Params params_{};
		PatternInfo parsed_{};

		// 秒取得
		std::function<float()> secondsGetter_{};

		// 直接指定モード用
		float secondsValue_ = 0.0f;
		bool useDirectSeconds_ = false;

		// キャッシュ文字列
		std::string cachedText_;
		// 最初のフレーム
		bool first_ = true;

		// エディター
		InputImGui inputText_{};

		//--------- functions ----------------------------------------------------

		// パターン解析
		std::string FormatTimeByPattern(float seconds);
		void ParsePattern(const std::string& pattern);
	};
} // SakuEngine
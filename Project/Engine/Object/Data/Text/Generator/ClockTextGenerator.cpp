#include "ClockTextGenerator.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

// imgui
#include <imgui.h>

//============================================================================
//	ClockTextGenerator classMethods
//============================================================================

namespace {

	// ゼロ埋め整数追加
	int32_t Pow10i(int32_t n) {

		// 10のn乗
		int32_t p = 1;
		for (int32_t i = 0; i < n; ++i) {

			p *= 10;
		}
		return p;
	}

	// ゼロ埋め整数追加
	void AppendZeroPaddedInt(std::string& out, int32_t value, int32_t width) {

		// マイナスはここでは扱わない
		if (value < 0) {
			value = -value;
		}

		// 桁数計算
		int32_t tmp = value;
		int32_t digits = 1;
		while (tmp >= 10) {
			tmp /= 10;
			++digits;
		}

		// widthより短い分だけ0埋め
		for (int32_t i = digits; i < width; ++i) {
			out.push_back('0');
		}

		// 数値を追加
		out += std::to_string(value);
	}
}

void ClockTextGenerator::SetSeconds(float seconds) {

	secondsValue_ = seconds;
	useDirectSeconds_ = true;
}

void ClockTextGenerator::SetSecondsGetter(std::function<float()> getter) {

	secondsGetter_ = std::move(getter);
	useDirectSeconds_ = false;
}

bool ClockTextGenerator::Generate(std::string& outString) {

	float seconds = 0.0f;
	// 秒取得
	if (useDirectSeconds_) {

		// 直接指定モード
		seconds = secondsValue_;
	} else {
		// 秒指定がなければ何もしない
		if (!secondsGetter_) {
			return false;
		}
		// 外部から秒取得
		seconds = secondsGetter_();
	}

	// 指定フォーマットで文字列化
	std::string formatted = FormatTimeByPattern(seconds);
	// 変更チェック
	if (first_ || formatted != cachedText_) {

		first_ = false;
		cachedText_ = formatted;
		outString = cachedText_;
		return true;
	}
	return false;
}

std::string ClockTextGenerator::FormatTimeByPattern(float seconds) {

	// 区切りグループ数
	const uint32_t groupCount = static_cast<uint32_t>(parsed_.groupWidths.size());
	if (groupCount <= 0) {
		return "00:00.00";
	}

	// 負数判定
	bool negative = (seconds < 0.0f);
	float secondsAbs = std::abs(seconds);
	//不数の場合、clampするなら0.0fにする
	if (negative && params_.clampNegativeToZero) {
		negative = false;
		secondsAbs = 0.0f;
	}

	// 整数秒、小数秒
	float secFloorD = std::floor(secondsAbs);
	int32_t totalSeconds = static_cast<int32_t>(secFloorD);

	// 小数部
	float frac = secondsAbs - secFloorD;
	if (frac < 0.0f) {
		frac = 0.0f;
	}
	if (frac > 1.0f) {
		frac = 1.0f;
	}

	// 小数部有無
	const bool hasFraction = parsed_.hasFraction;
	int32_t fractionWidth = 0;
	int32_t fractionValue = 0;

	// 小数部処理
	int32_t groupsNoFraction = groupCount;
	if (hasFraction) {

		// 小数部の最後の桁数
		fractionWidth = parsed_.groupWidths.back();
		groupsNoFraction = groupCount - 1;

		// 小数を指定桁で整数化
		const int32_t base = Pow10i(fractionWidth);
		float f = frac * static_cast<float>(base);
		// 四捨五入
		if (params_.roundFraction) {

			f += 0.5f;
		}

		// 桁溢れしたら繰り上げ
		fractionValue = static_cast<int32_t>(std::floor(f));
		if (base <= fractionValue) {

			// リセットして繰り上げ
			fractionValue = 0;
			totalSeconds += 1;
		}
	}

	// 時間要素を右側から割り当てる
	int32_t secPart = 0;
	int32_t minPart = 0;
	int32_t hourPart = 0;
	int32_t dayPart = 0;
	// グループ数に応じて分割
	if (groupsNoFraction <= 1) {

		secPart = totalSeconds;
	} else if (groupsNoFraction == 2) {

		secPart = totalSeconds % 60;
		minPart = totalSeconds / 60;
	} else if (groupsNoFraction == 3) {

		secPart = totalSeconds % 60;
		minPart = (totalSeconds / 60) % 60;
		hourPart = totalSeconds / 3600;
	} else {

		secPart = totalSeconds % 60;
		minPart = (totalSeconds / 60) % 60;
		hourPart = (totalSeconds / 3600) % 24;
		dayPart = totalSeconds / 86400;
	}

	std::string out;
	out.reserve(32);

	// 符号
	if (negative && params_.showMinusSign) {
		out.push_back('-');
	}

	// 左から文字列組み立て（）

	// 数値配列を左から右で作る
	std::vector<int32_t> parts;
	parts.reserve(groupCount);

	// 左から順に追加
	if (groupsNoFraction <= 1) {

		parts.push_back(secPart);

	} else if (groupsNoFraction == 2) {

		parts.push_back(minPart);
		parts.push_back(secPart);

	} else if (groupsNoFraction == 3) {

		parts.push_back(hourPart);
		parts.push_back(minPart);
		parts.push_back(secPart);

	} else {

		parts.push_back(dayPart);
		parts.push_back(hourPart);
		parts.push_back(minPart);
		parts.push_back(secPart);
	}

	if (hasFraction) {

		parts.push_back(fractionValue);
	}


	// 文字列組み立て
	int32_t actualGroups = static_cast<int32_t>(parts.size());
	for (int32_t gi = 0; gi < actualGroups; ++gi) {

		AppendZeroPaddedInt(out, parts[gi], parsed_.groupWidths[gi]);
		if (gi < actualGroups - 1) {

			// セパレータの数が足りない場合の保険
			if (gi < static_cast<int32_t>(parsed_.seps.size())) {

				out += parsed_.seps[gi];
			}
		}
	}
	return out;
}

void ClockTextGenerator::ParsePattern(const std::string& pattern) {

	// 解析結果初期化
	parsed_.groupWidths.clear();
	parsed_.seps.clear();
	parsed_.hasFraction = false;

	int32_t i = 0;
	const int32_t n = static_cast<int32_t>(pattern.size());

	while (i < n) {

		// 'd' の連続を探す
		if (pattern[i] == 'd') {

			int32_t w = 0;
			while (i < n && pattern[i] == 'd') {
				++w;
				++i;
			}
			parsed_.groupWidths.push_back(w);
			continue;
		}

		// セパレータはgroupの間にしか入れない
		// 次の'd'まで貯める
		std::string sep;
		while (i < n && pattern[i] != 'd') {
			sep.push_back(pattern[i]);
			++i;
		}

		// 最初のセパレータは無視
		if (!parsed_.groupWidths.empty() && !sep.empty()) {
			parsed_.seps.push_back(sep);
		}
	}
	if (!parsed_.groupWidths.empty()) {
		if (static_cast<int32_t>(parsed_.seps.size()) > static_cast<int32_t>(parsed_.groupWidths.size()) - 1) {

			parsed_.seps.resize(parsed_.groupWidths.size() - 1);
		}
	}

	// 小数部を持つか
	parsed_.hasFraction = (!parsed_.seps.empty() && parsed_.seps.back() == ".");

	// デフォルトパターン
	if (parsed_.groupWidths.empty()) {

		parsed_.groupWidths = { 2, 2, 2 };
		parsed_.seps = { ":", "." };
		parsed_.hasFraction = true;
	}
}

void ClockTextGenerator::ImGui() {

	// パターン入力
	if (ImGuiHelper::InputText("Pattern", inputText_)) {

		params_.pattern = inputText_.input;
	}
	if (ImGui::Button("Parse Pattern")) {

		ParsePattern(params_.pattern);
	}
	ImGui::Separator();
	ImGui::Spacing();
	// その他オプション
	ImGui::Checkbox("clampNegativeToZero", &params_.clampNegativeToZero);
	ImGui::Checkbox("showMinusSign", &params_.showMinusSign);
	ImGui::Checkbox("roundFraction", &params_.roundFraction);

	if (ImGui::CollapsingHeader("Debug Param")) {

		// パース結果表示
		// "dd"の個数リスト
		for (size_t i = 0; i < parsed_.groupWidths.size(); ++i) {
			ImGui::Text("Group %zu width: %d", i, parsed_.groupWidths[i]);
		}
		// 区切り文字
		for (size_t i = 0; i < parsed_.seps.size(); ++i) {
			ImGui::Text("Sep %zu: '%s'", i, parsed_.seps[i].c_str());
		}
		ImGui::Text(std::format("hasFraction: {}", parsed_.hasFraction).c_str());
	}

	ImGui::SeparatorText("Display");

	// 秒数指定
	ImGui::DragFloat("seconds", &secondsValue_, 0.01f);
	ImGui::Checkbox("useDirectSeconds", &useDirectSeconds_);

	// 外部秒数取得関数の有無
	if (secondsGetter_) {

		ImGui::Text("Using external seconds: %.3f", secondsGetter_());
	} else {

		ImGui::Text("No external seconds getter set");
	}
}
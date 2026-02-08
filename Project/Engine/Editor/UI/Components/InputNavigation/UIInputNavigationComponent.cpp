#include "UIInputNavigationComponent.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Editor/UI/Components/InputNavigation/Devices/UIKeyboardNavigationDevice.h>
#include <Engine/Editor/UI/Components/InputNavigation/Devices/UIGamepadNavigationDevice.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UIInputNavigationComponent classMethods
//============================================================================

void UIInputNavigationComponent::EnsureMapper() {

	if (mapper) {
		return;
	}

	// 初期化してデバイスを設定
	mapper = std::make_unique<InputMapper<UINavigationAction>>();
	mapper->AddDevice(std::make_unique<UIKeyboardNavigationDevice>(this));
	mapper->AddDevice(std::make_unique<UIGamepadNavigationDevice>(this));

	// デフォルトのデバイス入力設定
	// キーボード
	upKeys.emplace_back(static_cast<uint8_t>(DIK_W));
	upKeys.emplace_back(static_cast<uint8_t>(DIK_UP));
	downKeys.emplace_back(static_cast<uint8_t>(DIK_S));
	downKeys.emplace_back(static_cast<uint8_t>(DIK_DOWN));
	leftKeys.emplace_back(static_cast<uint8_t>(DIK_A));
	leftKeys.emplace_back(static_cast<uint8_t>(DIK_LEFT));
	rightKeys.emplace_back(static_cast<uint8_t>(DIK_D));
	rightKeys.emplace_back(static_cast<uint8_t>(DIK_RIGHT));
	submitKeys.emplace_back(static_cast<uint8_t>(DIK_RETURN));
	cancelKeys.emplace_back(static_cast<uint8_t>(DIK_ESCAPE));
	// ゲームパッド
	upPad.emplace_back(GamePadButtons::ARROW_UP);
	downPad.emplace_back(GamePadButtons::ARROW_DOWN);
	leftPad.emplace_back(GamePadButtons::ARROW_LEFT);
	rightPad.emplace_back(GamePadButtons::ARROW_RIGHT);
	submitPad.emplace_back(GamePadButtons::A);
	cancelPad.emplace_back(GamePadButtons::B);
}

void UIInputNavigationComponent::ImGui([[maybe_unused]] const ImVec2& itemSize) {

	ImGui::Checkbox("acceptInput", &acceptInput);
	ImGui::Checkbox("mouseHoverFocus", &mouseHoverFocus);
	ImGui::Checkbox("mouseClickSubmit", &mouseClickSubmit);
	ImGui::SliderFloat("stickThreshold", &stickThreshold, 0.0f, 1.0f);
	ImGui::SliderFloat("repeatDelaySecond", &repeatDelaySecond, 0.0f, 1.0f);
	ImGui::SliderFloat("repeatIntervalSecond", &repeatIntervalSecond, 0.01f, 0.5f);
}

void UIInputNavigationComponent::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	acceptInput = data.value("acceptInput", false);
	mouseHoverFocus = data.value("mouseHoverFocus", false);
	mouseClickSubmit = data.value("mouseClickSubmit", false);

	stickThreshold = data.value("stickThreshold", 0.5f);
	repeatDelaySecond = data.value("repeatDelaySecond", 0.25f);
	repeatIntervalSecond = data.value("repeatIntervalSecond", 0.1f);

	// キー配列の読み込み
	auto loadKeys = [&](const char* name, std::vector<uint8_t>& out) {

		out.clear();
		if (!data.contains(name)) {
			return;
		}
		for (auto& value : data[name]) {
			out.push_back(static_cast<uint8_t>(value.get<int>()));
		}
		};
	loadKeys("upKeys", upKeys);
	loadKeys("downKeys", downKeys);
	loadKeys("leftKeys", leftKeys);
	loadKeys("rightKeys", rightKeys);
	loadKeys("submitKeys", submitKeys);
	loadKeys("cancelKeys", cancelKeys);

	// ゲームパッドボタン配列の読み込み
	auto loadPad = [&](const char* name, std::vector<GamePadButtons>& out) {

		out.clear();
		if (!data.contains(name)) {
			return;
		}
		for (auto& value : data[name]) {
			auto optional = EnumAdapter<GamePadButtons>::FromString(value.get<std::string>());
			if (optional.has_value()) {
				out.push_back(*optional);
			}
		}
		};
	loadPad("upPad", upPad);
	loadPad("downPad", downPad);
	loadPad("leftPad", leftPad);
	loadPad("rightPad", rightPad);
	loadPad("submitPad", submitPad);
	loadPad("cancelPad", cancelPad);
}

void UIInputNavigationComponent::ToJson(Json& data) {

	data["acceptInput"] = acceptInput;
	data["mouseHoverFocus"] = mouseHoverFocus;
	data["mouseClickSubmit"] = mouseClickSubmit;

	data["stickThreshold"] = stickThreshold;
	data["repeatDelaySecond"] = repeatDelaySecond;
	data["repeatIntervalSecond"] = repeatIntervalSecond;

	// キー配列の保存
	auto saveKeys = [&](const char* name, const std::vector<uint8_t>& in) {

		Json array = Json::array();
		for (auto key : in) {
			array.push_back(static_cast<int>(key));
		}
		data[name] = array;
		};
	saveKeys("upKeys", upKeys);
	saveKeys("downKeys", downKeys);
	saveKeys("leftKeys", leftKeys);
	saveKeys("rightKeys", rightKeys);
	saveKeys("submitKeys", submitKeys);
	saveKeys("cancelKeys", cancelKeys);

	// ゲームパッドボタン配列の保存
	auto savePad = [&](const char* name, const std::vector<GamePadButtons>& in) {

		Json array = Json::array();
		for (auto button : in) {
			array.push_back(EnumAdapter<GamePadButtons>::ToString(button));
		}
		data[name] = array;
		};
	savePad("upPad", upPad);
	savePad("downPad", downPad);
	savePad("leftPad", leftPad);
	savePad("rightPad", rightPad);
	savePad("submitPad", submitPad);
	savePad("cancelPad", cancelPad);
}
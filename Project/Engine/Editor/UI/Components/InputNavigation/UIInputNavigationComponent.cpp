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

void UIInputNavigationComponent::ImGui(const ImVec2& itemSize) {

	ImGui::Checkbox("acceptInput", &acceptInput);
	ImGui::Checkbox("mouseHoverFocus", &mouseHoverFocus);
	ImGui::Checkbox("mouseClickSubmit", &mouseClickSubmit);
	ImGui::SliderFloat("stickThreshold", &stickThreshold, 0.0f, 1.0f);
	ImGui::SliderFloat("repeatDelaySecond", &repeatDelaySecond, 0.0f, 1.0f);
	ImGui::SliderFloat("repeatIntervalSecond", &repeatIntervalSecond, 0.01f, 0.5f);

	// 配列ごとの選択状態を保持
	static std::unordered_map<uintptr_t, int> s_selectedIndex;
	// キー追加用
	auto DrawKeyMappingButton = [&](const char* addLabel, std::vector<uint8_t>& keyArray) {

		ImGui::PushID(reinterpret_cast<void*>(&keyArray));

		// 左クリック: 追加
		if (ImGui::Button(addLabel, itemSize)) {
			// 同じキーが登録されていなければ
			bool alreadyExists = false;
			for (const auto existingKey : keyArray) {
				if (existingKey == static_cast<uint8_t>(addKeyDIKCode)) {

					alreadyExists = true;
					break;
				}
			}
			if (!alreadyExists) {

				keyArray.emplace_back(static_cast<uint8_t>(addKeyDIKCode));
			}
		}

		// 右クリック: 一覧ポップアップ
		if (ImGui::BeginPopupContextItem("##KeyList", ImGuiPopupFlags_MouseButtonRight)) {

			ImGui::Text("Count: %d", static_cast<int>(keyArray.size()));
			ImGui::Separator();

			int& selected = s_selectedIndex[reinterpret_cast<uintptr_t>(&keyArray)];
			if (selected >= static_cast<int>(keyArray.size())) {
				selected = -1;
			}

			constexpr float kListHeight = 160.0f;
			ImGui::BeginChild("##KeyListChild", ImVec2(itemSize.x / 2.0f, kListHeight), true);

			for (int i = 0; i < static_cast<int>(keyArray.size()); ++i) {

				ImGui::PushID(i);

				const auto code = static_cast<KeyDIKCode>(keyArray[i]);
				const std::string name = EnumAdapter<KeyDIKCode>::ToString(code);

				std::string label = std::to_string(i) + ": " +
					(name.empty() ? std::to_string(static_cast<int>(keyArray[i])) : name);

				// 左クリック: 選択
				if (ImGui::Selectable(label.c_str(), selected == i)) {
					selected = i;
				}

				// 右クリック: Removeボタン
				if (ImGui::BeginPopupContextItem("##KeyItemCtx", ImGuiPopupFlags_MouseButtonRight)) {

					ImGui::TextUnformatted(label.c_str());
					ImGui::Separator();

					if (ImGui::Button("Remove")) {

						keyArray.erase(keyArray.begin() + i);

						if (selected == i) {
							selected = -1;
						} else if (selected > i) {
							selected -= 1;
						}

						ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
						ImGui::PopID();
						// 削除したので抜ける
						break;
					}
					ImGui::EndPopup();
				}
				ImGui::PopID();
			}
			ImGui::EndChild();
			ImGui::Separator();

			// 全削除ボタン
			if (ImGui::Button("Clear All")) {

				keyArray.clear();
				selected = -1;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
		};
	// ゲームパッド追加用
	auto DrawGamePadMappingButton = [&](const char* addLabel, std::vector<GamePadButtons>& buttonArray) {

		ImGui::PushID(reinterpret_cast<void*>(&buttonArray));

		// 左クリック: 追加
		if (ImGui::Button(addLabel, itemSize)) {
			// 同じボタンが登録されていなければ
			bool alreadyExists = false;
			for (const auto existingButton : buttonArray) {
				if (existingButton == addGamePadButton) {

					alreadyExists = true;
					break;
				}
			}
			if (!alreadyExists) {

				buttonArray.emplace_back(addGamePadButton);
			}
		}

		// 右クリック: 一覧ポップアップ
		if (ImGui::BeginPopupContextItem("##PadList", ImGuiPopupFlags_MouseButtonRight)) {

			ImGui::Text("Count: %d", static_cast<int>(buttonArray.size()));
			ImGui::Separator();

			int& selected = s_selectedIndex[reinterpret_cast<uintptr_t>(&buttonArray)];
			if (selected >= static_cast<int>(buttonArray.size())) {
				selected = -1;
			}

			constexpr float kListHeight = 160.0f;
			ImGui::BeginChild("##PadListChild", ImVec2(260.0f, kListHeight), true);

			for (int i = 0; i < static_cast<int>(buttonArray.size()); ++i) {

				ImGui::PushID(i);

				const std::string name = EnumAdapter<GamePadButtons>::ToString(buttonArray[i]);
				std::string label = std::to_string(i) + ": " + (name.empty() ? "<Unknown>" : name);

				// 左クリック: 選択
				if (ImGui::Selectable(label.c_str(), selected == i)) {
					selected = i;
				}

				// 右クリック: Removeボタン
				if (ImGui::BeginPopupContextItem("##PadItemCtx", ImGuiPopupFlags_MouseButtonRight)) {

					ImGui::TextUnformatted(label.c_str());
					ImGui::Separator();

					if (ImGui::Button("Remove")) {

						buttonArray.erase(buttonArray.begin() + i);

						if (selected == i) {
							selected = -1;
						} else if (selected > i) {
							selected -= 1;
						}

						ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
						ImGui::PopID();
						// 削除したので抜ける
						break;
					}
					ImGui::EndPopup();
				}
				ImGui::PopID();
			}
			ImGui::EndChild();
			ImGui::Separator();

			// 全削除ボタン
			if (ImGui::Button("Clear All")) {

				buttonArray.clear();
				selected = -1;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
		};

	// キーボード
	ImGui::SeparatorText("Key Mappings");
	ImGui::PushID("KeyArray");

	EnumAdapter<KeyDIKCode>::Combo("addKey", &addKeyDIKCode);
	DrawKeyMappingButton("Add Up", upKeys);
	DrawKeyMappingButton("Add Down", downKeys);
	DrawKeyMappingButton("Add Left", leftKeys);
	DrawKeyMappingButton("Add Right", rightKeys);
	DrawKeyMappingButton("Add Submit", submitKeys);
	DrawKeyMappingButton("Add Cancel", cancelKeys);

	ImGui::PopID();

	// ゲームパッド
	ImGui::SeparatorText("GamePad Mappings");
	ImGui::PushID("GamePadArray");

	EnumAdapter<GamePadButtons>::Combo("addGamePad", &addGamePadButton);
	DrawGamePadMappingButton("Add Up", upPad);
	DrawGamePadMappingButton("Add Down", downPad);
	DrawGamePadMappingButton("Add Left", leftPad);
	DrawGamePadMappingButton("Add Right", rightPad);
	DrawGamePadMappingButton("Add Submit", submitPad);
	DrawGamePadMappingButton("Add Cancel", cancelPad);

	ImGui::PopID();
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
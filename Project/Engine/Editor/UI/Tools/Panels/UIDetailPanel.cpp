#include "UIDetailPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	UIDetailPanel classMethods
//============================================================================

namespace {

	// std::string用InputText
	static bool InputTextStdString(const char* label, std::string& str) {

		auto callback = [](ImGuiInputTextCallbackData* data)->int {

			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {

				auto* s = static_cast<std::string*>(data->UserData);
				s->resize(data->BufTextLen);
				data->Buf = s->data();
			}
			return 0;
			};
		if (str.capacity() < 64) {
			str.reserve(64);
		}
		return ImGui::InputText(label, str.data(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, callback, &str);
	}
}

void UIDetailPanel::ImGui(UIToolContext& context) {

	ImGui::SetWindowFontScale(0.72f);

	// 選択中のUIアセットを取得
	UIAsset* asset = context.GetSelectedAsset();
	if (!asset) {
		ImGui::Text("No UIAsset selected.");
		return;
	}
	// 未選択チェック
	if (!context.selectedElement.IsValid()) {
		ImGui::Text("No UIElement selected.");
		return;
	}

	// 選択中のUI要素を取得
	UIElement* element = asset->Get(context.selectedElement);
	if (!element) {
		ImGui::Text("Invalid element.");
		return;
	}

	const float itemWidth = 100.0f;

	//============================================================================
	//	エレメントの保存、読み込み
	//============================================================================
	{
		std::string outRelPath{};

		// 読み込み
		if (ImGui::Button("Load##UIElement", ImVec2(itemWidth, 28.0f))) {

			if (ImGuiHelper::OpenJsonDialog(outRelPath)) {

				Json data{};
				if (JsonAdapter::LoadCheck(outRelPath, data)) {

					// 読み込み処理
					asset->ImportJsonElementPrefab(data, context.selectedElement);
				}
			}
		}
		ImGui::SameLine();
		// 保存
		if (ImGui::Button("Save##UIElement", ImVec2(itemWidth, 28.0f))) {

			jsonSaveElementState_.showPopup = true;
		}
		// 実際の保存処理
		if (ImGuiHelper::SaveJsonModal("Save UIElement", UIElement::kBaseJsonPath.c_str(),
			UIElement::kBaseJsonPath.c_str(), jsonSaveElementState_, outRelPath)) {

			Json data{};
			asset->ExportJsonElementPrefab(data, context.selectedElement);
			JsonAdapter::Save(outRelPath, data);
		}
	}
	//============================================================================
	//	要素情報の表示、編集
	//============================================================================

	ImGui::SeparatorText("Element");

	InputTextStdString("name", element->name);
	ImGui::Text("children: %zu", element->children.size());

	ImGui::Separator();
	ImGui::Spacing();

	//============================================================================
	//	コンポーネントの表示、編集
	//============================================================================

	ImGui::SeparatorText("Components");

	// コンポーネント一覧
	for (auto componentHandle : element->components) {

		// コンポーネントを取得
		IUIComponent* component = asset->GetComponent(componentHandle);
		if (!component) {
			continue;
		}

		// コンポーネント名を取得
		const char* componentName = EnumAdapter<UIComponentType>::ToString(component->GetType());
		// 折りたたみヘッダーで表示
		if (ImGui::CollapsingHeader(componentName, ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::PushID(componentName);

			//============================================================================
			//	コンポーネントの保存、読み込み
			//============================================================================
			{
				std::string outRelPath{};

				// 読み込み
				if (ImGui::Button("Load##UIComponent", ImVec2(itemWidth, 28.0f))) {

					if (ImGuiHelper::OpenJsonDialog(outRelPath)) {

						Json data{};
						if (JsonAdapter::LoadCheck(outRelPath, data)) {

							// 読み込み処理
							component->FromJson(data);
						}
					}
				}
				ImGui::SameLine();
				// 保存
				if (ImGui::Button("Save##UIComponent", ImVec2(itemWidth, 28.0f))) {

					jsonSaveComponentState_.showPopup = true;
				}
				// 実際の保存処理
				if (ImGuiHelper::SaveJsonModal("Save UIComponent", UIComponentSlot::kBaseJsonPath.c_str(),
					UIComponentSlot::kBaseJsonPath.c_str(), jsonSaveComponentState_, outRelPath)) {

					Json data{};
					component->ToJson(data);
					JsonAdapter::Save(outRelPath, data);
				}
			}
			ImGui::Separator();

			// 各コンポーネントのImGuiを呼ぶ
			component->ImGui(ImVec2(192.0f, 28.0f));

			ImGui::PopID();
			ImGui::Separator();
			ImGui::Spacing();
		}
	}

	ImGui::SetWindowFontScale(1.0f);
}
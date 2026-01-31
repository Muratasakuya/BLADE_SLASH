#include "UIDetailPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>

// imgui
#include <imgui.h>
#include <imgui_internal.h>

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

	float itemWidth = 100.0f;

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
	ImGui::Text("uid:        %d", element->uid);
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

	//============================================================================
	//	コンポーネントの追加
	//============================================================================

	ImGui::Separator();
	ImGui::Spacing();

	itemWidth = ImGui::GetContentRegionAvail().x;

	// 追加候補
	struct AddEntry {

		const char* category;
		UIComponentType type;
		bool uniquePerElement; // 同一Elementに複数不可
		bool uniquePerAsset;   // Asset内で1つだけ
		bool rootOnly;         // Root限定
	};
	// 追加候補リスト
	static const AddEntry kAddEntries[] = {

		{ "Interaction", UIComponentType::Selectable,          true,  false, false },
		{ "Input",       UIComponentType::InputNavigation,     true,  true,  true  },
	};
	// 選択中エレメントにすでにあるか
	auto HasOnElement = [&](UIComponentType t) -> bool {
		return asset->FindComponent(context.selectedElement, t) != nullptr;
		};
	// アセット内にすでにあるか
	auto HasInAsset = [&](UIComponentType t) -> bool {
		bool found = false;
		asset->elements.ForEachAlive([&](UIElement::Handle handle, [[maybe_unused]] const UIElement& e) {
			if (found) {
				return;
			}
			if (asset->FindComponent(handle, t)) {
				found = true;
			}
			});
		return found;
		};

	// ボタンで表示、非表示切り替え
	{
		// ボタンラベル
		if (ImGui::Button("Add Component", ImVec2(itemWidth, 28.0f))) {

			showAddComponent_ = !showAddComponent_;
			if (showAddComponent_) {
				addComponentFilter_.Clear();
			}
		}
	}

	// 展開ブラウザ
	if (showAddComponent_) {

		// 見やすいように枠付き、背景色を少し変える
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

		// “ブラウザ”領域
		const float height = 320.0f;
		if (ImGui::BeginChild("##AddComponentBrowser", ImVec2(0.0f, height), true)) {

			// ヘッダ行：タイトル + 閉じる
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Add Component");
			ImGui::SameLine();

			// 右寄せでCloseボタン
			float closeW = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2.0f;
			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - closeW);
			if (ImGui::SmallButton("Close")) {

				showAddComponent_ = false;
			}

			// 検索
			addComponentFilter_.Draw("Search", -1.0f);
			ImGui::Separator();

			// リスト
			if (ImGui::BeginChild("##AddComponentList", ImVec2(0, 0), false)) {

				const char* currentCategory = nullptr;
				for (const auto& entry : kAddEntries) {

					const char* typeName = EnumAdapter<UIComponentType>::ToString(entry.type);

					// フィルタ
					if (!addComponentFilter_.PassFilter(typeName) && !addComponentFilter_.PassFilter(entry.category)) {
						continue;
					}

					// カテゴリ見出し
					if (!currentCategory || std::strcmp(currentCategory, entry.category) != 0) {

						currentCategory = entry.category;
						ImGui::SeparatorText(currentCategory);
					}

					// 追加可能か
					bool canAdd = true;
					if (entry.uniquePerElement && HasOnElement(entry.type)) {
						canAdd = false;
					}
					if (entry.uniquePerAsset && HasInAsset(entry.type)) {
						canAdd = false;
					}
					if (entry.rootOnly && !UIElement::Handle::Equal(context.selectedElement, asset->rootHandle)) {
						canAdd = false;
					}

					ImGui::BeginDisabled(!canAdd);
					ImGui::PushID((int)entry.type);

					// 左揃えのリスト
					if (ImGui::Selectable(typeName, false, ImGuiSelectableFlags_SpanAvailWidth)) {

						// 追加処理
						asset->AddComponentByType(context.selectedElement, entry.type);
						showAddComponent_ = false;
					}

					ImGui::PopID();
					ImGui::EndDisabled();
				}

				// list
				ImGui::EndChild();
			}
		}
		// browser
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
	}

	ImGui::SetWindowFontScale(1.0f);
}
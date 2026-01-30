#include "UIAssetPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	UIAssetPanel classMethods
//============================================================================

void UIAssetPanel::ImGui(UIToolContext& context) {

	const float itemWidth = 200.0f;

	//============================================================================
	//	UI名を入力して追加
	//============================================================================
	{
		// サイズ設定
		ImGui::PushItemWidth(itemWidth);

		// 新しいUIの名前
		ImGuiHelper::InputText("Name", inputText_);

		ImGui::PopItemWidth();

		// ボタンで追加
		if (ImGui::Button("Create UI", ImVec2(itemWidth, 28.0f))) {

			// 名前が空の場合は追加できない
			if (!inputText_.inputText.empty()) {
				// ライブラリに追加
				UIAssetHandle handle = context.assetLibrary->Add(inputText_.inputText);
				// 選択中にする
				context.selectedAsset = handle;

				// 選択されたUIのルートを選択中要素にする
				if (UIAsset* asset = context.assetLibrary->GetAsset(handle)) {

					context.selectedElement = asset->rootHandle;
				}
			}
		}
	}
	ImGui::Spacing();
	ImGui::Separator();
	//============================================================================
	//	UIの保存(選択しているUI)、読み込み
	//============================================================================
	{
		std::string outRelPath{};

		// 読み込み
		if (ImGui::Button("Load##UIAsset", ImVec2(itemWidth / 2.0f, 28.0f))) {

			if (ImGuiHelper::OpenJsonDialog(outRelPath)) {

				// 読み込み処理
			}
		}
		ImGui::SameLine();
		// 保存
		if (ImGui::Button("Save##UIAsset", ImVec2(itemWidth / 2.0f, 28.0f))) {

			jsonSaveState_.showPopup = true;
		}
		// 実際の保存処理
		if (ImGuiHelper::SaveJsonModal("Save UIAsset", UIAsset::kBaseJsonPath.c_str(),
			UIAsset::kBaseJsonPath.c_str(), jsonSaveState_, outRelPath)) {

			UIAsset* asset = context.GetSelectedAsset();
			if (asset) {

				Json data{};
				asset->ToJson(data);
				JsonAdapter::Save(outRelPath, data);
			}
		}
	}
	//============================================================================
	//	追加されているUIリストの一覧の表示、選択
	//============================================================================
	{
		ImGui::Spacing();
		ImGui::SeparatorText("UI List");

		// 一覧表示
		context.assetLibrary->ForEachAsset([&](UIAssetHandle handle, const UIAssetEntry& entry) {

			// 選択されているか
			bool selected = (context.selectedAsset.index == handle.index) &&
				(context.selectedAsset.generation == handle.generation);

			// テーブル表示
			if (ImGui::Selectable(entry.name.c_str(), selected)) {

				context.selectedAsset = handle;
				context.selectedElement = entry.asset.rootHandle;
			}
			});
	}
	//============================================================================
	//	選択中のUIを削除する
	//============================================================================
	{
		// ボタンで削除
		if (ImGui::Button("Delete Selected", ImVec2(200.0f, 28.0f))) {

			// 有効なアセットのみ削除可
			if (context.selectedAsset.IsValid()) {

				// 削除して未選択状態にする
				context.assetLibrary->Remove(context.selectedAsset);
				context.selectedAsset = {};
				context.selectedElement = {};
			}
		}
	}
}
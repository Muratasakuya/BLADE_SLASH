#include "UIAssetPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

//============================================================================
//	UIAssetPanel classMethods
//============================================================================

void UIAssetPanel::ImGui(UIToolContext& context) {

	//============================================================================
	//	UI名を入力して追加
	//============================================================================
	{
		const float itemWidth = 200.0f;
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
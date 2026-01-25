#include "UIHierarchyPanel.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Methods/UIPaletteRegistry.h>

// imgui
#include <imgui.h>

//============================================================================
//	UIHierarchyPanel classMethods
//============================================================================

void UIHierarchyPanel::ImGui(UIToolContext& context) {

	UIAsset* asset = context.GetSelectedAsset();
	if (!asset) {
		ImGui::Text("No UIAsset selected.");
		return;
	}

	// ルートノードから再帰的に描画
	DrawNodeRecursive(*asset, asset->rootHandle, context);
}

void UIHierarchyPanel::DrawNodeRecursive(UIAsset& asset, const UIElement::Handle& node, UIToolContext& context) {

	UIElement* element = asset.Get(node);
	if (!element) {
		return;
	}

	//============================================================================
	//	ツリーノード描画
	//============================================================================

	// 要素が選択されているか
	bool isSelected = (context.selectedElement.index == node.index) &&
		(context.selectedElement.generation == node.generation);

	// ノードフラグ設定
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanFullWidth |
		(element->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0) |
		(isSelected ? ImGuiTreeNodeFlags_Selected : 0);

	// ノード描画
	bool opened = ImGui::TreeNodeEx((void*)(intptr_t)node.index, flags, "%s", element->name.c_str());
	if (ImGui::IsItemClicked()) {

		// 選択
		context.selectedElement = node;
	}

	//============================================================================
	//	ヒエラルキー内ドラッグの移動元
	//============================================================================

	// ルート以外のノードのみドラッグ可能にする
	if (!UIElement::Handle::Equal(node, asset.rootHandle)) {

		// ノードをドラッグできるようにする
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

			// ペイロードにノードハンドルを設定
			UIElement::Handle payloadHandle = node;
			ImGui::SetDragDropPayload(kHierarchyPayloadId, &payloadHandle, sizeof(payloadHandle));

			ImGui::Text("Move : %s", element->name.c_str());
			ImGui::EndDragDropSource();
		}
	}

	//============================================================================
	//	ドロップ処理
	//============================================================================

	if (ImGui::BeginDragDropTarget()) {

		//------------------------------------------------------------------------
		//	パレットからのドロップ(要素追加)
		//------------------------------------------------------------------------
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPalettePayloadId)) {

			const UIPaletteItemType type = *(const UIPaletteItemType*)payload->Data;
			const UIPaletteItem* paletteItem = context.paletteRegistry->FindItem(type);
			if (paletteItem) {

				// アイテム作成
				UIElement::Handle newElement = paletteItem->onCreate(asset, node);
				context.selectedElement = newElement;
			}
		}

		//------------------------------------------------------------------------
		//	ヒエラルキーからのドロップ(親入れ替え)
		//------------------------------------------------------------------------
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kHierarchyPayloadId)) {

			const UIElement::Handle child = *(const UIElement::Handle*)payload->Data;

			// 移動できるかチェック
			if (CanReparent(asset, child, node)) {

				// 親を付け替え
				asset.Reparent(child, node);
				// 移動した要素を選択状態にする
				context.selectedElement = child;
			}
		}

		ImGui::EndDragDropTarget();
	}

	//============================================================================
	//	ノード削除処理
	//============================================================================

	// 右クリックで削除アイテムを出す
	if (ImGui::BeginPopupContextItem()) {
		if (!UIElement::Handle::Equal(node, asset.rootHandle)) {
			if (ImGui::MenuItem("Delete")) {

				asset.DestroyRecursive(node);
				context.selectedElement = asset.rootHandle;
				ImGui::EndPopup();
				if (opened) {
					ImGui::TreePop();
				}
				return;
			}
		}
		ImGui::EndPopup();
	}

	//============================================================================
	//	ツリーノードの子ノード描画
	//============================================================================

	if (opened) {
		// 子ノードを再帰的に描画
		for (const auto& child : element->children) {

			DrawNodeRecursive(asset, child, context);
		}
		ImGui::TreePop();
	}
}

bool UIHierarchyPanel::CanReparent(UIAsset& asset, const UIElement::Handle& child, const UIElement::Handle& newParent) {

	// 無効ハンドルは無視
	if (!child.IsValid() || !newParent.IsValid()) {
		return false;
	}

	// ルートは動かせない
	if (UIElement::Handle::Equal(child, asset.rootHandle)) {
		return false;
	}

	// 自分自身にドロップ禁止
	if (UIElement::Handle::Equal(child, newParent)) {
		return false;
	}

	// 要素が生きているか
	if (!asset.elements.IsAlive(child) || !asset.elements.IsAlive(newParent)) {
		return false;
	}

	// 子孫にはドロップできないようにする(循環して挙動がバグる)
	if (IsDescendant(asset, child, newParent)) {
		return false;
	}
	return true;
}

bool UIHierarchyPanel::IsDescendant(UIAsset& asset, const UIElement::Handle& ancestor, const UIElement::Handle& node) {

	// ノードの親を辿っていき、祖に当たったら子孫
	UIElement* current = asset.Get(node);
	if (!current) {
		return false;
	}

	UIElement::Handle parent = current->parentHandle;
	// 親が存在する限り辿る
	while (asset.elements.IsAlive(parent)) {

		// 祖に当たったら子孫
		if (UIElement::Handle::Equal(parent, ancestor)) {
			return true;
		}

		UIElement* parentElement = asset.Get(parent);
		if (!parentElement) {
			break;
		}
		parent = parentElement->parentHandle;
	}
	return false;
}
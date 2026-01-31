#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>

namespace SakuEngine {

	//============================================================================
	//	UIHierarchyPanel class
	//	UIの階層構造を表示、編集するパネル
	//============================================================================
	class UIHierarchyPanel :
		public IUIToolPanel {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIHierarchyPanel() = default;
		~UIHierarchyPanel() = default;

		// エディター
		void ImGui(UIToolContext& context) override;

		//--------- accessor -----------------------------------------------------

		// エディタの名前、パネルに表示する
		const char* GetName() const override { return "Hierarchy##UIEditor"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// ノードを再帰的に描画
		void DrawNodeRecursive(UIAsset& asset, const UIElement::Handle& node, UIToolContext& context);

		// 親子付け替えが可能かどうか
		bool CanReparent(UIAsset& asset, const UIElement::Handle& child, const UIElement::Handle& newParent);

		// 祖の子孫にノードが含まれているか
		bool IsDescendant(UIAsset& asset, const UIElement::Handle& ancestor, const UIElement::Handle& node);
	};
} // SakuEngine
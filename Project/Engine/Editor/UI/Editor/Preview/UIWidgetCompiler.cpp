#include "UIWidgetCompiler.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Document/UIWidgetDocument.h>
#include <Engine/Editor/UI/UISlateWidget/Mehtods/UIWidgetFactory.h>
#include <Engine/Editor/UI/UISlateWidget/Core/UISlateWidget.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/UISlatePanelWidget.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateImage.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateTextBlock.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateButton.h>
#include <Engine/Editor/UI/UISlateWidget/Widgets/Basic/UISlateSlider.h>

//============================================================================
//	UIWidgetCompiler classMethods
//============================================================================

std::unique_ptr<UISlateWidget> UIWidgetCompiler::BuildRuntimeTree(
	const UIWidgetDocument& document) const {

	const uint32_t rootId = document.GetRootId();
	// ルートノードが無ければnullptrを返す
	if (rootId == 0) {
		return nullptr;
	}
	// ノードを構築を構築して返す
	return BuildNodeRecursive(document, rootId);
}

std::unique_ptr<UISlateWidget> UIWidgetCompiler::BuildNodeRecursive(
	const UIWidgetDocument& document, uint32_t nodeId) const {

	const UIWidgetNode* node = document.FindNode(nodeId);
	if (!node) {
		return nullptr;
	}

	// ノード生成
	auto widget = UIWidgetFactory::GetInstance().Create(node->typeName);
	if (!widget) {
		return nullptr;
	}

	// 固有情報設定
	widget->SetName(node->name);
	widget->SetId(node->id);
	widget->SetGeneration(1);

	// 共通設定
	ApplyCommonProperty(document, nodeId, *widget);
	ApplyTypeProperty(document, nodeId, *widget);

	// パネルなら子ノードを追加
	if (widget->IsPanel()) {
		if (auto* panel = static_cast<UISlatePanelWidget*>(widget.get())) {
			for (auto child : node->children) {
				if (auto newChild = BuildNodeRecursive(document, child)) {

					panel->AddChild(std::move(newChild));
				}
			}
		}
	}
	return widget;
}

void UIWidgetCompiler::ApplyCommonProperty(const UIWidgetDocument& document,
	uint32_t nodeId, UISlateWidget& widget) const {

	const UIWidgetNode* node = document.FindNode(nodeId);
	if (!node) {
		return;
	}

	// 可視性設定
	widget.SetEnabled(node->enabled);
	widget.SetVisibility(node->visibility);

	// レイアウト設定
	auto& layout = widget.GetLayout();
	layout = node->layout;
}

void UIWidgetCompiler::ApplyTypeProperty(const UIWidgetDocument& document,
	uint32_t nodeId, UISlateWidget& widget) const {

	const UIWidgetNode* node = document.FindNode(nodeId);
	if (!node) {
		return;
	}
	
	// UISlateImageプロパティ設定
	if (auto* image = dynamic_cast<UISlateImage*>(&widget)) {

		image->SetTextureName(document.GetProperty<std::string>(nodeId, "textureName", "white"));
		return;
	}

	// UISlateTextBlockプロパティ設定
	if (auto* textBlock = dynamic_cast<UISlateTextBlock*>(&widget)) {

		textBlock->SetText(document.GetProperty<std::string>(nodeId, "text", "Text"));
		textBlock->SetTextBindingKey(document.GetProperty<std::string>(nodeId, "textBindingKey", ""));
		return;
	}

	// UISlateButtonプロパティ設定
	if (auto* button = dynamic_cast<UISlateButton*>(&widget)) {

		UIButtonStyle style{};
		style.normalTexture = document.GetProperty<std::string>(nodeId, "style.normal", "");
		style.hoveredTexture  = document.GetProperty<std::string>(nodeId, "style.hovered", "");
		style.pressedTexture = document.GetProperty<std::string>(nodeId, "style.pressed", "");
		style.disabledTexture = document.GetProperty<std::string>(nodeId, "style.disabled", "");
		button->SetButtonStyle(style);
		button->SetClickActionKey(document.GetProperty<std::string>(nodeId, "clickActionKey", ""));
		return;
	}

	// UISlateSliderプロパティ設定
	if (auto* slider = dynamic_cast<UISlateSlider*>(&widget)) {

		slider->SetValue(document.GetProperty<float>(nodeId, "value", 0.0f));
		slider->SetValueBindingKey(document.GetProperty<std::string>(nodeId, "valueBindingKey", ""));
		return;
	}
}
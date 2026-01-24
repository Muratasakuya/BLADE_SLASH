#include "UISlateCanvasPanel.h"

using namespace SakuEngine;

//============================================================================
//	UISlateCanvasPanel classMethods
//============================================================================

void UISlateCanvasPanel::ArrangeChildren(const UIRect& allotted, std::vector<UIArrangedWidget>& outArranged) {

	// クリア
	outArranged.clear();
	// 自分のキャッシュ
	SetCachedRect(allotted);

	// 子を配置
	for (auto& children : children_) {

		if (!children) {
			continue;
		}

		// 子の可視状態確認
		if (children->GetVisibility() == UIVisibility::Collapsed) {
			continue;
		}

		// 子の矩形計算
		UIRect childRect = ComputeRectFromAnchorData(allotted, children->GetLayout());
		children->SetCachedRect(childRect);

		// 配置情報登録
		UIArrangedWidget arrangedWidget{};
		arrangedWidget.widget = children.get();
		arrangedWidget.rect = childRect;
		arrangedWidget.zOrder = 0;
		outArranged.push_back(arrangedWidget);

		// パネルの子も再帰的に配置
		if (children->IsPanel()) {

			std::vector<UIArrangedWidget> dummy{};
			children->ArrangeChildren(childRect, dummy);
		}
	}
}

void UISlateCanvasPanel::FromJson(const Json& data) {

	UISlatePanelWidget::FromJson(data);
}

void UISlateCanvasPanel::ToJson(Json& data) {

	UISlatePanelWidget::ToJson(data);
}
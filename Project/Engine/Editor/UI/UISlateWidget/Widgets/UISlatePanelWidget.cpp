#include "UISlatePanelWidget.h"

using namespace SakuEngine;

//============================================================================
//	UISlatePanelWidget classMethods
//============================================================================

void UISlatePanelWidget::AddChild(std::unique_ptr<UISlateWidget> child) {

	children_.push_back(std::move(child));
}

void UISlatePanelWidget::Tick(UIUserWidget& owner, float deltaTime) {

	// 全ての子のフレーム更新
	for (auto& children : children_) {

		children->Tick(owner, deltaTime);
	}
}

void UISlatePanelWidget::OnAddedToTree(UIUserWidget& owner) {

	for (auto& children : children_) {

		children->OnAddedToTree(owner);
	}
}

void UISlatePanelWidget::SynchProperties(UIUserWidget& owner) {

	for (auto& children : children_) {

		children->SynchProperties(owner);
	}
}

void UISlatePanelWidget::PaintDrawData(UIUserWidget& owner) {

	for (auto& children : children_) {

		children->PaintDrawData(owner);
	}
}

void UISlatePanelWidget::GetChildren(std::vector<UISlateWidget*>& outChildren) {

	// クリアしてから再取得
	outChildren.clear();
	outChildren.reserve(children_.size());
	for (auto& children : children_) {

		outChildren.push_back(children.get());
	}
}

void UISlatePanelWidget::FromJson(const Json& data) {

	UISlateWidget::FromJson(data);
}

void UISlatePanelWidget::ToJson(Json& data) {

	UISlateWidget::ToJson(data);

	// 全ての子のデータを保存
	Json array = Json::array();
	for (auto& children : children_) {

		Json child{};
		children->ToJson(child);
		array.push_back(child);
	}
	data["children"] = array;
}
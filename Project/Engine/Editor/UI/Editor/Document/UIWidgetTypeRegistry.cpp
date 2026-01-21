#include "UIWidgetTypeRegistry.h"

using namespace SakuEngine;

//============================================================================
//	UIWidgetTypeRegistry classMethods
//============================================================================

void UIWidgetTypeRegistry::Register(const UIWidgetTypeInfo& info) {

	// 既に登録されている場合は無視
	auto it = map_.find(info.type);
	if (it != map_.end()) {
		return;
	}
	// 登録
	map_[info.type] = all_.size();
	all_.push_back(info);
}

std::vector<UIWidgetTypeInfo> UIWidgetTypeRegistry::GetByCategory(const std::string& category) const {

	// カテゴリー別に抽出して返す
	std::vector<UIWidgetTypeInfo> out;
	for (auto& type : all_) {
		if (type.category == category) {
			out.push_back(type);
		}
	}
	return out;
}

const UIWidgetTypeInfo* UIWidgetTypeRegistry::Find(const std::string& type) const {

	// タイプ名から検索
	auto it = map_.find(type);
	if (it == map_.end()) {
		return nullptr;
	}
	return &all_[it->second];
}

void UIWidgetTypeRegistry::RegisterDefaults() {

	// デフォルトウィジェット型登録
	// 親を持てるパネル
	Register({ "CanvasPanel", "Canvas Panel", "Layout", true });
	Register({ "Button", "Button", "Common", true });
	Register({ "Slider", "Slider", "Common", true });
	// 親を持てない表示系
	Register({ "Image", "Image", "Common", false });
	Register({ "TextBlock", "Text Block", "Common", false });
}
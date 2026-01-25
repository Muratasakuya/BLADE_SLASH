#include "UIPaletteRegistry.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UIPaletteRegistry classMethods
//============================================================================

const UIPaletteItem* UIPaletteRegistry::FindItem(UIPaletteItemType type) const {

	for (const auto& item : items_) {
		if (item.type == type) {

			return &item;
		}
	}
	return nullptr;
}

void UIPaletteRegistry::RegisterDefaultItems() {

	// アイテムを追加
	// パネル系
	AddSimpleItem(UIPaletteItemType::ImageButton, UIPaletteItemCategory::Panel, "UIImageButtonIcon");
	AddSimpleItem(UIPaletteItemType::TextButton, UIPaletteItemCategory::Panel, "UITextButtonIcon");
	// リーフ系
	AddSimpleItem(UIPaletteItemType::Text, UIPaletteItemCategory::Leaf, "UITextIcon");
	AddSimpleItem(UIPaletteItemType::Image, UIPaletteItemCategory::Leaf, "UIImageIcon");
	AddSimpleItem(UIPaletteItemType::CheckBox, UIPaletteItemCategory::Leaf, "UICheckBoxIcon");
	AddSimpleItem(UIPaletteItemType::ProgressBar, UIPaletteItemCategory::Leaf, "UIProgressBarIcon");
	AddSimpleItem(UIPaletteItemType::Slider, UIPaletteItemCategory::Leaf, "UISliderIcon");
}

void UIPaletteRegistry::AddSimpleItem(UIPaletteItemType type,
	UIPaletteItemCategory category, const std::string& iconName) {

	UIPaletteItem item{};
	// アイテムを追加
	item.type = type;
	item.category = category;
	item.iconName = iconName;
	// 作成コールバックを設定
	item.onCreate = [type](UIAsset& asset, HandlePool<UIElement>::Handle parent)->
		HandlePool<UIElement>::Handle {

		// 要素名を取得
		std::string typeName = EnumAdapter<UIPaletteItemType>::ToString(type);

		// 要素を追加
		UIElement element{};
		element.name = typeName;
		UIElement::Handle handle = asset.elements.Emplace(element);

		// 子要素として登録
		asset.AddChild(parent, handle);
		return handle;
		};
	// アイテムを登録
	AddItem(item);
}
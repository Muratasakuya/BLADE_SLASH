#include "UIPaletteRegistry.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UIPaletteRegistry classMethods
//============================================================================

void UIPaletteRegistry::RegisterDefaultItems() {

	// アイテムを追加
	// パネル系
	AddSimpleItem(UIPaletteItemType::Button, UIPaletteItemCategory::Panel);
	AddSimpleItem(UIPaletteItemType::TextButton, UIPaletteItemCategory::Panel);
	// リーフ系
	AddSimpleItem(UIPaletteItemType::Text, UIPaletteItemCategory::Leaf);
	AddSimpleItem(UIPaletteItemType::Image, UIPaletteItemCategory::Leaf);
	AddSimpleItem(UIPaletteItemType::CheckBox, UIPaletteItemCategory::Leaf);
	AddSimpleItem(UIPaletteItemType::ProgressBar, UIPaletteItemCategory::Leaf);
	AddSimpleItem(UIPaletteItemType::Slider, UIPaletteItemCategory::Leaf);
}

void UIPaletteRegistry::AddSimpleItem(UIPaletteItemType type, UIPaletteItemCategory category) {

	AddItem(UIPaletteItem{
		// タイプ、カテゴリを設定
		.type = type,
		.category = category,
		// 作成コールバックを設定
		.onCreate = [type](UIAsset& asset, UIElement::Handle parent) {

			// 要素を追加
			UIElement::Handle element = asset.elements.Emplace(UIElement{.name = EnumAdapter<UIPaletteItemType>::ToString(type) });
			// 子要素として登録
			asset.AddChild(parent, element);
			return element;
		}
		});
}
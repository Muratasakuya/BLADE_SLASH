#include "UIPaletteRegistry.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Transform/UISpriteTransformComponent.h>
#include <Engine/Editor/UI/Component/Transform/UITextTransformComponent.h>
#include <Engine/Editor/UI/Component/Transform/UIParentRectTransform.h>
#include <Engine/Editor/UI/Component/Sprite/UISpriteComponent.h>
#include <Engine/Editor/UI/Component/Text/UITextComponent.h>
#include <Engine/Editor/UI/Component/Selectable/UISelectableComponent.h>
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
	AddSimpleItem(UIPaletteItemType::SpriteButton, UIPaletteItemCategory::Panel, "UIImageButtonIcon");
	AddSimpleItem(UIPaletteItemType::TextButton, UIPaletteItemCategory::Panel, "UITextButtonIcon");
	// リーフ系
	AddSimpleItem(UIPaletteItemType::Text, UIPaletteItemCategory::Leaf, "UITextIcon");
	AddSimpleItem(UIPaletteItemType::Sprite, UIPaletteItemCategory::Leaf, "UIImageIcon");
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
	item.onCreate = [type, this](UIAsset& asset, UIElement::Handle parent)->UIElement::Handle {

		// 要素名を取得
		std::string typeName = EnumAdapter<UIPaletteItemType>::ToString(type);
		typeName = typeName;

		// 要素を追加
		UIElement element{};
		element.name = typeName;
		UIElement::Handle handle = asset.elements.Emplace(element);
		// 子要素として登録
		asset.AddChild(parent, handle);

		// コンポーネントを追加
		AddComponents(type, handle, asset);

		return handle;
		};
	// アイテムを登録
	AddItem(item);
}

void UIPaletteRegistry::AddComponents(UIPaletteItemType type, const UIElement::Handle& ownerRootHandle, UIAsset& asset) {

	// ルート要素に親矩形トランスフォームを追加
	asset.AddComponent<UIParentRectTransform>(ownerRootHandle);
	// トランスフォームを初期化
	auto* transform = static_cast<UIParentRectTransform*>(asset.FindComponent(ownerRootHandle, UIComponentType::ParentRectTransform));
	transform->transform.Init(nullptr);
	transform->transform.UpdateMatrix();

	switch (type) {
	case UIPaletteItemType::SpriteButton: {

		// 表示スプライト要素追加
		AddSpriteComponent(EnumAdapter<UIPaletteItemType>::ToString(type), ownerRootHandle, asset);
		break;
	}
	case UIPaletteItemType::TextButton: {

		// 表示テキスト要素を追加
		AddTextComponent(EnumAdapter<UIPaletteItemType>::ToString(type), ownerRootHandle, asset);
		break;
	}
	case UIPaletteItemType::Text: {
		
		// 表示テキスト要素を追加
		AddTextComponent(EnumAdapter<UIPaletteItemType>::ToString(type), ownerRootHandle, asset);
		break;
	}
	case UIPaletteItemType::Sprite: {

		// 表示スプライト要素追加
		AddSpriteComponent(EnumAdapter<UIPaletteItemType>::ToString(type), ownerRootHandle, asset);
		break;
	}
	case UIPaletteItemType::CheckBox: {

		// ✓マークの背景
		AddSpriteComponent("CheckBox_Background", ownerRootHandle, asset);
		// ✓マーク本体
		AddSpriteComponent("CheckBox_Check", ownerRootHandle, asset);
		break;
	}
	case UIPaletteItemType::ProgressBar: {

		// 進捗バー背景
		AddSpriteComponent("ProgressBar_Background", ownerRootHandle, asset);
		// 実際の値に応じた進捗
		AddSpriteComponent("ProgressBar_Progress", ownerRootHandle, asset);
		break;
	}
	case UIPaletteItemType::Slider: {

		// スライダー全体背景
		AddSpriteComponent("Slider_Background", ownerRootHandle, asset);
		// スライダー値の背景
		AddSpriteComponent("Slider_ValueHighlight", ownerRootHandle, asset);
		// スライダー値、持ち手
		AddSpriteComponent("Slider_ValueHand", ownerRootHandle, asset);
		break;
	}
	}
}

void UIPaletteRegistry::AddSpriteComponent(const std::string& name, const UIElement::Handle& parentHandle, UIAsset& asset) {

	UIElement sprite{};
	sprite.name = name;
	UIElement::Handle handle = asset.elements.Emplace(sprite);
	// 子要素として追加
	asset.AddChild(parentHandle, handle);

	// コンポーネント追加
	asset.AddComponent<UISpriteTransformComponent>(handle);
	asset.AddComponent<UISpriteComponent>(handle);
}

void UIPaletteRegistry::AddTextComponent(const std::string& name, const UIElement::Handle& parentHandle, UIAsset& asset) {

	UIElement text{};
	text.name = name;
	UIElement::Handle handle = asset.elements.Emplace(text);
	// 子要素として追加
	asset.AddChild(parentHandle, handle);

	// コンポーネント追加
	asset.AddComponent<UITextTransformComponent>(handle);
	asset.AddComponent<UITextComponent>(handle);
}
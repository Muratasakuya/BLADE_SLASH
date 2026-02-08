#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/AssetData/UIAsset.h>

// c++
#include <string>
#include <functional>

namespace SakuEngine {

	//============================================================================
	//	UIPaletteRegistry structures
	//============================================================================

	// アイテムタイプ
	enum class UIPaletteItemType {

		ParentRectTransform, // 親矩形トランスフォーム
		SpriteButton,        // 画像ボタン(Spriteを一つ生成する)
		TextButton,          // テキストボタン(Textを一つ生成する)
		Text,                // テキスト(Textを一つ生成する)
		Sprite,              // 画像(Spriteを一つ生成する)
		CheckBox,            // チェックボックス(背景用、✓用に二つSpriteを生成する)
		ProgressBar,         // プログレスバー(背景用、全体用に二つSpriteを生成する)
		Slider,              // スライダー(背景用、全体、持ち手用に三つSpriteを生成する)
	};

	// アイテムカテゴリ
	enum class UIPaletteItemCategory {

		Panel, // パネル、子を持つ
		Leaf,  // 要素単体、子を持たない
	};

	// UIパレットのアイテム情報
	struct UIPaletteItem {

		UIPaletteItemType type;         // タイプ
		UIPaletteItemCategory category; // カテゴリ

		// アイコン画像の名前
		std::string iconName;

		// 作成時のコールバック関数
		std::function<UIElement::Handle(UIAsset& asset, UIElement::Handle parent)> onCreate;
	};

	//============================================================================
	//	UIPaletteRegistry class
	//	UIパレットの登録を行うクラス
	//============================================================================
	class UIPaletteRegistry {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIPaletteRegistry() = default;
		~UIPaletteRegistry() = default;

		// アイテムの登録
		void RegisterDefaultItems();

		//--------- accessor -----------------------------------------------------

		// 親矩形トランスフォームを自動で追加するか設定
		void SetAutoAddParentRectTransform(bool value) { autoAddParentRectTransform_ = value; }
		// 親矩形トランスフォームを自動で追加するか取得
		bool IsAutoAddParentRectTransform() const { return autoAddParentRectTransform_; }

		// タイプからアイテムを検索して返す、見つからなければnullptr
		const UIPaletteItem* FindItem(UIPaletteItemType type) const;
		// 登録されているアイテムリストを返す
		const std::vector<UIPaletteItem>& GetItems() const { return items_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// アイテムリスト
		std::vector<UIPaletteItem> items_;

		// 親矩形トランスフォームを自動で追加するか
		bool autoAddParentRectTransform_ = false;

		//--------- functions ----------------------------------------------------

		// アイテムの追加
		void AddItem(const UIPaletteItem& item) { items_.emplace_back(item); }
		void AddSimpleItem(UIPaletteItemType type, UIPaletteItemCategory category, const std::string& iconName);

		// コンポーネントの追加
		UIElement::Handle AddComponents(UIPaletteItemType type, const UIElement::Handle& ownerRootHandle, UIAsset& asset);
		// スプライト
		UIElement::Handle AddSpriteComponent(const std::string& name, const UIElement::Handle& parentHandle, UIAsset& asset);
		// テキスト
		UIElement::Handle AddTextComponent(const std::string& name, const UIElement::Handle& parentHandle, UIAsset& asset);
	};
} // SakuEngine
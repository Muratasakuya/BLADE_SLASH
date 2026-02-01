#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/AssetData/UIAssetLibrary.h>
#include <Engine/Editor/UI/Animation/UIAnimationLibrary.h>

namespace SakuEngine {

	// front
	class Asset;
	class UIPaletteRegistry;

	//============================================================================
	//	UIToolContext class
	//	UIツールのコンテキスト情報を管理するクラス
	//============================================================================
	struct UIToolContext {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIToolContext() = default;
		~UIToolContext() = default;

		//--------- variables ----------------------------------------------------

		// リソースアセット
		Asset* asset = nullptr;
		// UIアセットライブラリ
		UIAssetLibrary* assetLibrary = nullptr;
		// UIアニメーションライブラリ
		UIAnimationLibrary* animationLibrary = nullptr;
		// UIパレットレジストリ
		UIPaletteRegistry* paletteRegistry = nullptr;

		// 選択中のUIアセットハンドル
		UIAssetHandle selectedAsset{};
		UIElement::Handle selectedElement{};

		//--------- accessor -----------------------------------------------------

		// 選択中のUIアセットを返す
		UIAsset* GetSelectedAsset() const { return assetLibrary->GetAsset(selectedAsset); }
	};
} // SakuEngine
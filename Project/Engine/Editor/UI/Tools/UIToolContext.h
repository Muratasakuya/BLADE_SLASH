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

		// 選択中のUIアセット、要素
		UIAssetHandle selectedAsset{};
		UIElement::Handle selectedElement{};

		// プレビュー関連
		// プレビュー有効フラグ
		bool previewEnabled = true;
		// プレビュー中のアニメーションクリップUID
		uint32_t previewClipUid = 0;
		// プレビュースタート要求フラグ
		bool previewStart = false;
		// プレビュー時間
		bool enablePreviewLoop = true;
		float previewLoopTime = 1.0f;

		//--------- accessor -----------------------------------------------------

		// 選択中のUIアセットを返す
		UIAsset* GetSelectedAsset() const { return assetLibrary->GetAsset(selectedAsset); }
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/AssetData/UIAsset.h>
#include <Engine/Object/Data/Canvas/CanvasCommon.h>
#include <Engine/MathLib/MathUtils.h>

// c++
#include <functional>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationAccess structures
	//============================================================================

	// UIプロパティアクセスのための関数定義
	template <typename T>
	struct UIPropertyAccess {

		std::function<T()> getter;            // プロパティの取得関数
		std::function<void(const T&)> setter; // プロパティの設定関数

		// 有効チェック
		bool Valid() const { return getter && setter; }
	};

	// 座標
	UIPropertyAccess<Vector2> BuildTranslationAccess(UIAsset& asset, const UIElement::Handle& handle, CanvasType canvasType);
	// 回転
	UIPropertyAccess<float> BuildRotationAccess(UIAsset& asset, const UIElement::Handle& handle, CanvasType canvasType);
	// スケール
	UIPropertyAccess<Vector2> BuildScaleAccess(UIAsset& asset, const UIElement::Handle& handle, CanvasType canvasType);
	// 色
	UIPropertyAccess<Color> BuildColorAccess(UIAsset& asset, const UIElement::Handle& handle, CanvasType canvasType);
} // SakuEngine
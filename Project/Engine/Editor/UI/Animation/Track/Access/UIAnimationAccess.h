#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/UIAnimationTarget.h>
#include <Engine/Editor/UI/AssetData/UIAsset.h>
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

	// Vector2プロパティアクセスの構築
	UIPropertyAccess<Vector2> BuildVec2Access(UIAsset& asset, const UIElement::Handle& handle, UIAnimationTargetType target, UIAnimationProperty property);
	// floatプロパティアクセスの構築
	UIPropertyAccess<float> BuildFloatAccess(UIAsset& asset, const UIElement::Handle& handle, UIAnimationTargetType target, UIAnimationProperty property);
	// Colorプロパティアクセスの構築
	UIPropertyAccess<Color> BuildColorAccess(UIAsset& asset, const UIElement::Handle& handle, UIAnimationTargetType target, UIAnimationProperty property);
} // SakuEngine
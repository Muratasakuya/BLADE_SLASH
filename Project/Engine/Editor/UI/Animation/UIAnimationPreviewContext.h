#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/AssetData/UIAsset.h>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationPreviewContext structure
	//	UIアニメーションプレビューのコンテキスト情報を管理する構造
	//============================================================================

	struct UIAnimationPreviewContext {

		// プレビュー有効フラグ
		bool enabled = false;
		// プレビュー中のアニメーションクリップUID
		uint32_t clipUid = 0;
		// プレビュー対象のUI要素ハンドル
		UIElement::Handle element{};

		// アニメーション開始要求フラグ
		bool requestStart = false;
	};
} // SakuEngine
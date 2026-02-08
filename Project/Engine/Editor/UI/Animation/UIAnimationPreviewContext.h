#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/AssetData/UIAsset.h>
#include <Engine/Utility/Timer/StateTimer.h>

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

		// プレビュー対象の元アセット側UI要素ハンドル
		UIElement::Handle element{};
		// プレビュー対象の複製されたUI要素ハンドル
		UIElement::Handle previewElement{};

		// アニメーション開始要求フラグ
		bool requestStart = false;
		// 現在のシステム更新がプレビュー対象アセット更新パスかどうか
		bool isPreviewPass = false;
		// 元アセット側のUI要素のサブツリーを抑制するかどうか
		bool suppressOriginalSubtree = false;

		// プレビューループ用タイマー
		bool enablePreviewLoop = true;
		StateTimer previewTimer{};
	};
} // SakuEngine
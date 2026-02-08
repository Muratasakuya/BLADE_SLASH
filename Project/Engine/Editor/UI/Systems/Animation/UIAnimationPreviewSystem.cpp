#include "UIAnimationPreviewSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/UIAnimationLibrary.h>

//============================================================================
//	UIAnimationPreviewSystem classMethods
//============================================================================

void UIAnimationPreviewSystem::Update(UISystemContext* context, UIAsset& asset) {

	// プレビューが無効なら処理しない
	if (!context->preview.enabled ||
		context->preview.clipUid == 0 ||
		!context->preview.previewElement.IsValid()) {
		return;
	}

	// プレビュー再生処理
	Play(context, asset);

	// アニメーション更新
	player_.Update(asset, playingElement_);
}

void UIAnimationPreviewSystem::Play(UISystemContext* context, UIAsset& asset) {

	// ループ再生が有効、未再生の時
	if (context->preview.enablePreviewLoop && !player_.IsPlaying()) {

		context->preview.previewTimer.Update(std::nullopt, false);
		// タイマーが到達したら再生要求を出す
		if (context->preview.previewTimer.IsReached()) {

			context->preview.requestStart = true;
			context->preview.previewTimer.Reset();
		}
	}

	// アニメーション再生リクエストがある場合
	if (context->preview.requestStart) {

		// 再生中情報更新
		playingClipUid_ = context->preview.clipUid;
		playingElement_ = context->preview.previewElement;
		context->preview.requestStart = false;

		// クリップ取得と再生
		if (auto* clip = context->animationLibrary->GetClip(playingClipUid_)) {

			player_.Play(*clip, asset, playingElement_);
		}
	}
}
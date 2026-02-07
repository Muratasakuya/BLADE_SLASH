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

	// アニメーション更新
	player_.Update(asset, playingElement_);
}
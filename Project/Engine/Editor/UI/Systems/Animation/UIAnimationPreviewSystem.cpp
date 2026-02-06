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
		!context->preview.element.IsValid()) {
		return;
	}

	// クリップまたは要素が変更された
	if (context->preview.requestStart) {

		// 再生中情報更新
		playingClipUid_ = context->preview.clipUid;
		playingElement_ = context->preview.element;
		context->preview.requestStart = false;

		// クリップ取得と再生
		if (auto* clip = context->animationLibrary->GetClip(playingClipUid_)) {

			player_.Play(*clip, asset, playingElement_);
		}
	}

	// アニメーション更新
	player_.Update(asset, playingElement_);
}
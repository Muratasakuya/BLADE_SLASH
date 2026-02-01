#include "UIStateAnimationSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Selectable/UISelectableComponent.h>
#include <Engine/Editor/UI/Component/Animation/UIStateAnimationComponent.h>
#include <Engine/Editor/UI/Animation/Tracks/Factory/UIAnimationTrackFactory.h>
#include <Engine/Editor/UI/Animation/UIAnimationLibrary.h>

//============================================================================
//	UIStateAnimationSystem classMethods
//============================================================================

namespace {

	// 全トラックが終了しているか
	bool AllFinished(const UIStateAnimationComponent& animation) {

		for (const auto& track : animation.tracks) {
			if (track && !track->IsFinished()) {

				return false;
			}
		}
		return true;
	}
}

void UIStateAnimationSystem::Update(UISystemContext* context, UIAsset& asset) {

	// 全要素を走査して、状態に応じたアニメーションを再生する
	asset.elements.ForEachAlive([&](UIElement::Handle handle, [[maybe_unused]] const UIElement& element) {

		// コンポーネントを取得
		auto* selectable = static_cast<UISelectableComponent*>(asset.FindComponent(handle, UIComponentType::Selectable));
		auto* animation = static_cast<UIStateAnimationComponent*>(asset.FindComponent(handle, UIComponentType::StateAnimation));
		if (!selectable || !animation) {
			return;
		}

		// 状態に対応するアニメーションクリップUIDを取得
		uint32_t animationClipUid = animation->stateToClipUid[selectable->state];

		// 状態が変化したとき、再生クリップが変化したときにアニメーションを開始
		if (animation->lastState != selectable->state || animation->playingClipUid != animationClipUid) {

			// クリア、リセット
			animation->tracks.clear();
			animation->isPlaying = false;

			// UIDと状態を更新
			animation->playingClipUid = animationClipUid;
			animation->lastState = selectable->state;

			// クリップUIDが有効ならアニメーションを開始
			if (animationClipUid != 0) {
				if (auto* clip = context->animationLibrary->GetClip(animationClipUid)) {

					// トラックを生成
					animation->tracks.reserve(clip->tracks.size());
					for (const auto& track : clip->tracks) {
						if (auto newTrack = UIAnimationTrackFactory::Create(track)) {

							animation->tracks.emplace_back(std::move(newTrack));
						}
					}

					// トラックを開始
					for (auto& track : animation->tracks) {

						track->Start(asset, handle, clip->canvasType);
					}
					// 再生中フラグを立てる
					animation->isPlaying = true;
				}
			}
		}

		// 再生中ならトラックを更新
		if (animation->isPlaying) {
			if (auto* clip = context->animationLibrary->GetClip(animation->playingClipUid)) {
				for (auto& track : animation->tracks) {
					if (track) {

						track->Update(asset, handle, clip->canvasType);
					}
				}
				// 全トラックが終了しているか
				if (AllFinished(*animation)) {

					// 再生終了
					animation->isPlaying = false;
				}
			} else {

				// クリップが見つからない場合も再生終了にしておく
				animation->isPlaying = false;
			}
		}

		// アニメーションが終了している場合、状態を遷移させる
		if (!animation->isPlaying) {
			switch (selectable->state) {
			case UIElementState::ShowBegin: {

				selectable->state = UIElementState::Showing;
				break;
			}
			case UIElementState::ShowEnd: {

				selectable->state = UIElementState::Hidden;
				break;
			}
			}
		}
		});
}
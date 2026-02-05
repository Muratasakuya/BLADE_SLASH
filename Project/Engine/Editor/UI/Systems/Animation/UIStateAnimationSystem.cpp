#include "UIStateAnimationSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/Selectable/UISelectableComponent.h>
#include <Engine/Editor/UI/Components/Animation/UIStateAnimationComponent.h>
#include <Engine/Editor/UI/Animation/UIAnimationLibrary.h>

//============================================================================
//	UIStateAnimationSystem classMethods
//============================================================================

void UIStateAnimationSystem::Update(UISystemContext* context, UIAsset& asset) {

	// 全要素を走査して、状態に応じたアニメーションを再生する
	asset.elements.ForEachAlive([&](UIElement::Handle handle, const UIElement&) {

		// 必要なコンポーネントを取得
		auto* selectable = (UISelectableComponent*)asset.FindComponent(handle, UIComponentType::Selectable);
		auto* animation = (UIStateAnimationComponent*)asset.FindComponent(handle, UIComponentType::StateAnimation);
		if (!selectable || !animation) {
			return;
		}

		// 状態に応じたアニメーションクリップUIDを取得
		uint32_t clipUid = 0;
		if (auto it = animation->stateToClipUid.find(selectable->state); it != animation->stateToClipUid.end()) {
			clipUid = it->second;
		}

		// 状態が切り替わったかどうか
		const bool stateChanged = (animation->lastState != selectable->state);
		const bool clipChanged = (animation->playingClipUid != clipUid);
		if (stateChanged || clipChanged) {

			// 状態が切り替わった場合はアニメーションを再生し直す
			animation->lastState = selectable->state;
			animation->playingClipUid = clipUid;
			if (clipUid != 0) {
				if (auto* clip = context->animationLibrary->GetClip(clipUid)) {

					animation->player.Play(*clip, asset, handle);
				}
			}
		}

		// アニメーションの更新
		animation->player.Update(asset, handle);

		// 再生が終了していたら状態を更新
		if (!animation->player.IsPlaying()) {
			switch (selectable->state) {
			case UIElementState::ShowBegin: {

				selectable->state = UIElementState::Showing;
				break;
			}
			case UIElementState::ShowEnd: {

				selectable->state = UIElementState::Hidden;
			}
			}
		}
		});
}
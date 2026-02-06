#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/Interface/IUISystem.h>
#include <Engine/Editor/UI/Animation/UIAnimationPlayer.h>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationPreviewSystem class
	//	UIアニメーションプレビューシステムクラス
	//============================================================================
	class UIAnimationPreviewSystem :
		public IUISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIAnimationPreviewSystem() = default;
		~UIAnimationPreviewSystem() = default;

		void Update(UISystemContext* context, UIAsset& asset) override;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// アニメーションプレイヤー
		UIAnimationPlayer player_;

		// 再生中のクリップUID
		uint32_t playingClipUid_ = 0;
		// 再生中の要素ハンドル
		UIElement::Handle playingElement_{};
	};
} // SakuEngine
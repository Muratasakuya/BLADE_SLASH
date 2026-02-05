#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Track/Interface/IUIAnimationTrackRuntime.h>
#include <Engine/Editor/UI/Animation/UIAnimationClip.h>
#include <Engine/Editor/UI/AssetData/UIAsset.h>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationPlayer class
	//	UIアニメーションを再生するプレイヤークラス
	//============================================================================
	class UIAnimationPlayer {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIAnimationPlayer() = default;
		~UIAnimationPlayer() = default;

		// アニメーションの再生
		void Play(const UIAnimationClip& clip, UIAsset& asset, const  UIElement::Handle& handle);
		// アニメーションの更新
		void Update(UIAsset& asset, const UIElement::Handle& handle);

		//--------- accessor -----------------------------------------------------

		// 再生中かどうか
		bool IsPlaying() const { return playing_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// トラックランタイムリスト
		std::vector<std::unique_ptr<IUIAnimationTrackRuntime>> tracks_;

		// 再生中フラグ
		bool playing_ = false;

		//--------- functions ----------------------------------------------------

		// トラックアセットからランタイムを生成
		std::unique_ptr<IUIAnimationTrackRuntime> CreateRuntimeFromAsset(const UIAnimationTrackAsset& track);
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/UIAnimationClip.h>
#include <Engine/Editor/UI/AssetData/UIAsset.h>
#include <Engine/Editor/UI/Animation/Tracks/Access/UIAnimationAccess.h>

namespace SakuEngine {

	//============================================================================
	//	IUIAnimationTrackRuntime class
	//	UIアニメーションを再生する際のトラックランタイムインターフェース
	//============================================================================
	class IUIAnimationTrackRuntime {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IUIAnimationTrackRuntime() = default;
		virtual ~IUIAnimationTrackRuntime() = default;

		// アニメーションの開始
		virtual void Start(UIAsset& asset, const UIElement::Handle& elementHandle, CanvasType canvasType) = 0;
		// アニメーションの更新
		virtual void Update(UIAsset& asset, const UIElement::Handle& elementHandle, CanvasType canvasType) = 0;

		//--------- accessor -----------------------------------------------------

		// アニメーションの終了判定
		virtual bool IsFinished() const = 0;

		// 値の種類を返す
		virtual UIAnimationDataType GetType() const = 0;
	};
} // SakuEngine
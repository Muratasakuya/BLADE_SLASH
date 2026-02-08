#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/UIAnimationPreviewContext.h>

namespace SakuEngine {

	// front
	class UIAnimationLibrary;

	//============================================================================
	//	UISystemContext class
	//	UIシステムのコンテキスト情報を管理するクラス
	//============================================================================
	struct UISystemContext {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISystemContext() = default;
		~UISystemContext() = default;

		//--------- variables ----------------------------------------------------

		// UIアニメーションライブラリ
		UIAnimationLibrary* animationLibrary = nullptr;
		// UIアニメーションプレビューコンテキスト
		UIAnimationPreviewContext preview;
	};
} // SakuEngine
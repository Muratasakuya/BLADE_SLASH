#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/Interface/IUISystem.h>

namespace SakuEngine {

	//============================================================================
	//	UITextSyncSystem class
	//	UIテキストオブジェクト同期システム
	//============================================================================
	class UITextSyncSystem :
		public IUISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UITextSyncSystem() = default;
		~UITextSyncSystem() = default;

		// UIアセットを更新
		void Update(UISystemContext* context, UIAsset& asset) override;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// UIオブジェクトの同期処理を親から子まで再帰的に行う
		void UpdateRecursive(UISystemContext* context, UIAsset& asset, const UIElement::Handle& node,
			bool canSuppress, bool parentSuppressed);
		// テキストコンポーネント用のオブジェクトを作成する
		void EnsureTextObject(UIAsset& asset, const UIElement& element,
			const UIElement::Handle& node, bool suppressed);
	};
} // SakuEngine
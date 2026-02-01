#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/Interface/IUISystem.h>

namespace SakuEngine {

	//============================================================================
	//	UISpriteSyncSystem class
	//	UIスプライトオブジェクト同期システム
	//============================================================================
	class UISpriteSyncSystem :
		public IUISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISpriteSyncSystem() = default;
		~UISpriteSyncSystem() = default;

		// UIアセットを更新
		void Update(UISystemContext* context, UIAsset& asset) override;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// UIオブジェクトの同期処理を親から子まで再帰的に行う
		void UpdateRecursive(UIAsset& asset, const UIElement::Handle& node);
		// スプライトコンポーネント用のオブジェクトを作成する
		void EnsureSpriteObject(UIAsset& asset, const UIElement& element, const UIElement::Handle& node);
	};
} // SakuEngine
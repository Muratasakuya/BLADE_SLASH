#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/System/Interface/IUISystem.h>
#include <Engine/Editor/UI/Component/Sprite/UISpriteComponent.h>
#include <Engine/Editor/UI/Component/Transform/UISpriteTransformComponent.h>

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
		void Update(UIAsset& asset) override;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// UIオブジェクトの同期処理を親から子まで再帰的に行う
		void UpdateRecursive(UIAsset& asset, const UIElement::Handle& node);
		// トランスフォームデータの更新
		void ApplyTransform(uint32_t objectId, const UISpriteTransformComponent& component);
		// スプライトコンポーネント用のオブジェクトを作成する
		void EnsureSpriteObject(const UIElement& element, UISpriteComponent& component);
	};
} // SakuEngine
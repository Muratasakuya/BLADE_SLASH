#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/System/Interface/IUISystem.h>
#include <Engine/Editor/UI/Component/Text/UITextComponent.h>
#include <Engine/Editor/UI/Component/Transform/UITextTransformComponent.h>
#include <Engine/Editor/UI/Component/Transform/UIParentRectTransform.h>

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
		void Update(UIAsset& asset) override;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// UIオブジェクトの同期処理を親から子まで再帰的に行う
		void UpdateRecursive(UIAsset& asset, const UIElement::Handle& node);
		// トランスフォームデータの更新
		void ApplyTransform(uint32_t objectId, const UITextTransformComponent& component);
		// テキストデータの更新
		void ApplyText(uint32_t objectId, const UITextComponent& component);
		// テキストコンポーネント用のオブジェクトを作成する
		void EnsureTextObject(UIAsset& asset, const UIElement& element, UITextComponent& component);
	};
} // SakuEngine
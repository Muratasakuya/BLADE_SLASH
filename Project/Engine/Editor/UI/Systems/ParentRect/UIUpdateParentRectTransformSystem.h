#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/Interface/IUISystem.h>
#include <Engine/Editor/UI/Component/Transform/UIParentRectTransform.h>

namespace SakuEngine {

	//============================================================================
	//	UIUpdateParentRectTransformSystem class
	//	UI親要素のトランスフォーム更新システム
	//============================================================================
	class UIUpdateParentRectTransformSystem :
		public IUISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIUpdateParentRectTransformSystem() = default;
		~UIUpdateParentRectTransformSystem() = default;

		// UIアセットを更新
		void Update(UISystemContext* context, UIAsset& asset) override;

	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// 行列更新を親から子まで再帰的に行う
		void UpdateRecursive(UIAsset& asset, const UIElement::Handle& node, const UIParentRectTransform* parent);
	};
} // SakuEngine
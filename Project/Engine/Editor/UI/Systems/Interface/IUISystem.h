#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/AssetData/UIAsset.h>
#include <Engine/Editor/UI/Systems/UISystemContext.h>

namespace SakuEngine {

	//============================================================================
	//	IUISystem class
	//	UIを処理するシステムのインターフェース
	//============================================================================
	class IUISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IUISystem() = default;
		virtual ~IUISystem() = default;

		// UIアセットを更新
		virtual void Update(UISystemContext* context, UIAsset& asset) = 0;
	};

	// UIシステム用メソッド群
	namespace UISystemMethod {

		// コンポーネントのjsonキャッシュを基に復元
		void RestoreFromJsonCache(IUIComponent& component);
	}
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/Interface/IUISystem.h>

// c++
#include <vector>

namespace SakuEngine {

	//============================================================================
	//	UIRuntime class
	//	UIアセットをランタイム更新するクラス
	//============================================================================
	class UIRuntime {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIRuntime() = default;
		~UIRuntime() = default;

		// 初期化
		void Init();

		// UIアセットを更新
		void Update(UISystemContext* context, UIAsset& asset);
		// プレビュー
		void UpdatePreview(UISystemContext* context, UIAsset& asset);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// アセット更新を行うシステム群
		std::vector<std::unique_ptr<IUISystem>> systems_;
		// プレビュー用更新システム群
		std::vector<std::unique_ptr<IUISystem>> previewSystems_;
	};
} // SakuEngine
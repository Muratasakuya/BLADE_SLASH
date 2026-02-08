#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/Interface/IUISystem.h>
#include <Engine/Editor/UI/AssetData/UIAsset.h>

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
		void UpdatePreview(UISystemContext* context, UIAsset& originalAsset);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// アセット更新を行うシステム群
		std::vector<std::unique_ptr<IUISystem>> systems_;
		// プレビュー用更新システム群
		std::vector<std::unique_ptr<IUISystem>> previewSystems_;

		// プレビュー関連情報
		// プレビュー用UIアセット
		UIAsset previewAsset_{};
		// プレビュー用ルート要素ハンドル
		UIElement::Handle previewRoot_{};
		// プレビュー対象要素UID
		uint32_t previewSourceElementUid_ = 0;
		// プレビュー用要素が生存しているか
		bool previewAlive_ = false;

		//--------- functions ----------------------------------------------------

		// プレビューアセットを破棄
		void ClearPreviewAsset();
		// プレビューアセットを作り直す
		void RebuildPreviewAsset(UIAsset& originalAsset, UIElement::Handle originalElement);
	};
} // SakuEngine
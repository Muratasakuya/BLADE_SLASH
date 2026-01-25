#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/AssetData/UIAsset.h>

namespace SakuEngine {

	// アセット専用ハンドル
	using UIAssetPool = HandlePool<UIAssetEntry>;
	using UIAssetHandle = UIAssetPool::Handle;

	//============================================================================
	//	UIAssetLibrary class
	//	作成したUIアセットを管理するライブラリ
	//============================================================================
	class UIAssetLibrary {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIAssetLibrary() = default;
		~UIAssetLibrary() = default;

		// UIの追加
		UIAssetHandle Add(const std::string& name);
		// UIの削除
		void Remove(UIAssetHandle handle) { assets_.Destroy(handle); }

		//--------- accessor -----------------------------------------------------

		// 名前の取得
		std::string GetName(UIAssetHandle handle) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// UIアセットリスト
		UIAssetPool assets_;

		//--------- functions ----------------------------------------------------

	};
} // SakuEngine
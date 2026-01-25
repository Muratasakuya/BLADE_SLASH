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

		// 全てのUIアセットに対して関数を実行
		template<typename Func>
		void ForEachAsset(Func&& func);

		//--------- accessor -----------------------------------------------------

		// 名前の取得
		std::string GetName(UIAssetHandle handle) const;

		// UIアセットの取得
		UIAsset* GetAsset(UIAssetHandle handle);
		const UIAsset* GetAsset(UIAssetHandle handle) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// UIアセットリスト
		UIAssetPool assets_;
	};

	//============================================================================
	//	UIAssetLibrary templateMethods
	//============================================================================

	template<typename Func>
	inline void UIAssetLibrary::ForEachAsset(Func&& func) {

		assets_.ForEachAlive([&](UIAssetHandle handle, UIAssetEntry entry) {
			func(handle, entry);
			});
	}
} // SakuEngine
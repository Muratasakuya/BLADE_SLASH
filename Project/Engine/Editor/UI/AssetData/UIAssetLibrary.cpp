#include "UIAssetLibrary.h"

using namespace SakuEngine;

//============================================================================
//	UIAssetLibrary classMethods
//============================================================================

UIAssetHandle UIAssetLibrary::Add(const std::string& name) {

	// 新規UIアセットエントリを作成
	UIAssetHandle newAsset = assets_.Emplace();
	// エントリ情報を設定
	UIAssetEntry* entry = assets_.Get(newAsset);
	entry->name = name;
	entry->asset.Init();

	return newAsset;
}

std::string UIAssetLibrary::GetName(UIAssetHandle handle) const {

	const UIAssetEntry* entry = assets_.Get(handle);
	return entry ? entry->name : "Invalid_UIAsset";
}

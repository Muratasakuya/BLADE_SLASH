#include "UIAssetLibrary.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

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

void UIAssetLibrary::LoadAsset(const std::string& filePath) {

	Json data{};
	if (!JsonAdapter::LoadCheck(filePath, data)) {
		return;
	}

	// エントリを追加
	UIAssetHandle handle = assets_.Emplace();

	// エントリ情報を設定
	UIAssetEntry* entry = assets_.Get(handle);
	entry->name = data.value("Name", "Unknown");
	entry->asset.FromJson(data["UIAsset"]);
}

void UIAssetLibrary::SaveAsset(const UIAssetHandle handle, const std::string& filePath) {

	// 現在、エディター選択されているUIアセットを保存
	Json data{};

	if (UIAssetEntry* entry = assets_.Get(handle)) {

		data["Name"] = entry->name;
		entry->asset.ToJson(data["UIAsset"]);
	}

	JsonAdapter::Save(filePath, data);
}

std::string UIAssetLibrary::GetName(UIAssetHandle handle) const {

	const UIAssetEntry* entry = assets_.Get(handle);
	return entry ? entry->name : "Invalid_UIAsset";
}

UIAsset* UIAssetLibrary::GetAsset(UIAssetHandle handle) {

	// UIアセットエントリを取得
	UIAssetEntry* entry = assets_.Get(handle);
	return entry ? &entry->asset : nullptr;
}

const UIAsset* UIAssetLibrary::GetAsset(UIAssetHandle handle) const {

	// UIアセットエントリを取得
	const UIAssetEntry* entry = assets_.Get(handle);
	return entry ? &entry->asset : nullptr;
}
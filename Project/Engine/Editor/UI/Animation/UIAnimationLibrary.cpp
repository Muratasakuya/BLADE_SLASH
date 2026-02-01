#include "UIAnimationLibrary.h"

using namespace SakuEngine;

//============================================================================
//	UIAnimationLibrary classMethods
//============================================================================

UIAnimationHandle UIAnimationLibrary::Create(CanvasType canvasType) {

	// 新規UIアニメーションクリップエントリを作成
	UIAnimationEntry entry{};
	entry.clip.uid = AllocateUid();
	entry.clip.canvasType = canvasType;
	entry.clip.name = MakeUniqueName("Animation");
	entry.clip.tracks.clear();

	// エントリを追加
	UIAnimationHandle handle = clips_.Emplace(entry);
	uidToHandle_[entry.clip.uid] = handle;
	return handle;
}

void UIAnimationLibrary::LoadAllAnimations() {

	namespace fs = std::filesystem;

	uidToHandle_.clear();
	fs::path directory(UIAnimationClip::kBaseJsonPath);
	// ディレクトリが存在しない場合は終了
	if (!fs::exists(directory)) {
		return;
	}

	// ディレクトリ内のjsonファイルを全て読み込む
	for (auto& path : fs::directory_iterator(directory)) {

		// ファイルでなければスキップ
		if (!path.is_regular_file()) {
			continue;
		}
		// 拡張子が.jsonでなければスキップ
		if (path.path().extension() != ".json") {
			continue;
		}
		// ファイルを読み込む
		LoadAnimation(path.path().string());
	}
}

void UIAnimationLibrary::LoadAnimation(const std::string& filePath) {

	Json data{};
	if (!JsonAdapter::LoadCheck(filePath, data)) {
		return;
	}

	// エントリを追加
	UIAnimationEntry entry{};
	entry.filePath = filePath;
	entry.clip.FromJson(data["UIAnimation"]);

	// uidが無い古いデータ対策
	if (entry.clip.uid == 0) {

		entry.clip.uid = AllocateUid();
	}
	nextUid_ = (std::max)(nextUid_, entry.clip.uid + 1);

	// エントリを追加
	UIAnimationHandle handle = clips_.Emplace(entry);
	uidToHandle_[entry.clip.uid] = handle;
}

void UIAnimationLibrary::SaveAnimation(const UIAnimationHandle handle, const std::string& filePath) {

	// 指定ハンドルのアニメーションクリップを保存
	UIAnimationEntry* entry = clips_.Get(handle);
	if (!entry) {
		return;
	}

	// 保存
	Json data{};
	entry->clip.ToJson(data["UIAnimation"]);
	entry->filePath = filePath;
	JsonAdapter::Save(filePath, data);
}

void UIAnimationLibrary::ForEachClips(std::function<void(UIAnimationHandle, UIAnimationEntry&)> func) {

	clips_.ForEachAlive([&](UIAnimationHandle handle, UIAnimationEntry& entry) {
		func(handle, entry);
		});
}

UIAnimationClip* UIAnimationLibrary::GetClip(uint32_t uid) {

	// UIDからハンドルを引いて存在しなければnullptrを返す
	auto it = uidToHandle_.find(uid);
	if (it == uidToHandle_.end()) {
		return nullptr;
	}
	UIAnimationEntry* entry = clips_.Get(it->second);
	return entry ? &entry->clip : nullptr;
}

const UIAnimationClip* UIAnimationLibrary::GetClip(uint32_t uid) const {

	// UIDからハンドルを引いて存在しなければnullptrを返す
	auto it = uidToHandle_.find(uid);
	if (it == uidToHandle_.end()) {
		return nullptr;
	}
	const UIAnimationEntry* entry = clips_.Get(it->second);
	return entry ? &entry->clip : nullptr;
}

const std::string* UIAnimationLibrary::GetName(uint32_t uid) const {

	auto* clip = GetClip(uid);
	return clip ? &clip->name : nullptr;
}

std::string UIAnimationLibrary::MakeUniqueName(const std::string& base) {

	int32_t index = 0;
	while (true) {

		// 名前を生成
		std::string name = base + "_" + std::to_string(index);

		bool exists = false;
		clips_.ForEachAlive([&]([[maybe_unused]] UIAnimationHandle handle, UIAnimationEntry& entry) {

			// 同じ名前が存在するときはフラグを立ててインクリメントさせる
			if (entry.clip.name == name) {
				exists = true;
			}
			});
		// 存在しなければその名前を返す
		if (!exists) {
			return name;
		}
		++index;
	}
	return "";
}
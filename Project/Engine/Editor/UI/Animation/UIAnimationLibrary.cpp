#include "UIAnimationLibrary.h"

using namespace SakuEngine;

//============================================================================
//	UIAnimationLibrary classMethods
//============================================================================

UIAnimationHandle UIAnimationLibrary::Create(CanvasType canvasType) {

	return Create(canvasType, "Animation");
}

UIAnimationHandle UIAnimationLibrary::Create(CanvasType canvasType, const std::string& desiredName) {

	// 新規UIアニメーションクリップエントリを作成
	UIAnimationEntry entry{};
	entry.clip.uid = AllocateUid();
	entry.clip.canvasType = canvasType;

	// 希望名からユニーク化
	entry.clip.name = MakeUniqueNameFromRequested(desiredName.empty() ? "Animation" : desiredName, entry.clip.uid);
	entry.clip.tracks.clear();

	// エントリを追加
	UIAnimationHandle handle = clips_.Emplace(std::move(entry));
	uidToHandle_[entry.clip.uid] = handle;
	return handle;
}

bool UIAnimationLibrary::Destroy(uint32_t uid) {

	auto it = uidToHandle_.find(uid);
	if (it == uidToHandle_.end()) {
		return false;
	}

	// ハンドルを取得してエントリを削除
	const UIAnimationHandle handle = it->second;
	uidToHandle_.erase(it);
	clips_.Destroy(handle);
	return true;
}

bool UIAnimationLibrary::Rename(uint32_t uid, const std::string& desiredName) {

	if (desiredName.empty()) {
		return false;
	}

	UIAnimationClip* clip = GetClip(uid);
	if (!clip) {
		return false;
	}

	clip->name = MakeUniqueNameFromRequested(desiredName, uid);
	return true;
}

void UIAnimationLibrary::LoadAllAnimations() {

	namespace fs = std::filesystem;

	clips_.Clear();
	uidToHandle_.clear();
	nextUid_ = 1;
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

	// 名前が空なら最低限
	if (entry.clip.name.empty()) {

		entry.clip.name = MakeUniqueName("Animation");
	} else {

		// 既存名の重複がある場合もユニーク化
		entry.clip.name = MakeUniqueNameFromRequested(entry.clip.name, entry.clip.uid);
	}

	// エントリを追加
	UIAnimationHandle handle = clips_.Emplace(std::move(entry));
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

UIAnimationHandle UIAnimationLibrary::GetHandle(uint32_t uid) const {

	auto it = uidToHandle_.find(uid);
	if (it == uidToHandle_.end()) {
		return {};
	}
	return it->second;
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

std::string UIAnimationLibrary::MakeUniqueNameFromRequested(const std::string& requested, uint32_t exceptUid) {

	// requestedが未使用ならそのまま返す
	auto IsUsed = [&](const std::string& name) -> bool {

		bool used = false;
		clips_.ForEachAlive([&]([[maybe_unused]] UIAnimationHandle handle, UIAnimationEntry& entry) {
			if (entry.clip.uid == exceptUid) {
				return;
			}
			if (entry.clip.name == name) {
				used = true;
			}
			});
		return used;
		};

	if (!IsUsed(requested)) {
		return requested;
	}

	// 重複するならsuffixを付ける
	for (int32_t i = 1; i < 100000; ++i) {

		std::string candidate = requested + "_" + std::to_string(i);
		if (!IsUsed(candidate)) {

			return candidate;
		}
	}
	return MakeUniqueName(requested);
}
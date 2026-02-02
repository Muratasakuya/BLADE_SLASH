#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/UIAnimationClip.h>
#include <Engine/Utility/Algorithm/HandlePool.h>

// c++
#include <functional>

namespace SakuEngine {

	// アニメーション専用ハンドル
	using UIAnimationPool = HandlePool<UIAnimationEntry>;
	using UIAnimationHandle = UIAnimationPool::Handle;

	//============================================================================
	//	UIAnimationLibrary class
	//	作成したUIアニメーションを管理するライブラリ
	//============================================================================
	class UIAnimationLibrary {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIAnimationLibrary() = default;
		~UIAnimationLibrary() = default;

		// アニメーションの追加
		UIAnimationHandle Create(CanvasType canvasType);
		UIAnimationHandle Create(CanvasType canvasType, const std::string& desiredName);
		// アニメーションの削除
		bool Destroy(uint32_t uid);
		// 名前の変更
		bool Rename(uint32_t uid, const std::string& desiredName);

		// json
		void LoadAllAnimations();
		void LoadAnimation(const std::string& filePath);
		void SaveAnimation(const UIAnimationHandle handle, const std::string& filePath);

		// UIDの割り当て
		uint32_t AllocateUid() { return nextUid_++; }

		// 全てのアニメーションに対して関数を実行
		void ForEachClips(std::function<void(UIAnimationHandle, UIAnimationEntry&)> func);

		//--------- accessor -----------------------------------------------------

		// アニメーションクリップの取得
		UIAnimationClip* GetClip(uint32_t uid);
		const UIAnimationClip* GetClip(uint32_t uid) const;

		// ハンドルからエントリを取得
		UIAnimationEntry* GetEntry(UIAnimationHandle handle) { return clips_.Get(handle); }
		const UIAnimationEntry* GetEntry(UIAnimationHandle handle) const { return clips_.Get(handle); }

		// UIDから情報を取得
		UIAnimationHandle GetHandle(uint32_t uid) const;
		const std::string* GetName(uint32_t uid) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// UIアニメーションクリップリスト
		UIAnimationPool clips_;

		// UIDからハンドルを引くためのマップ
		std::unordered_map<uint32_t, UIAnimationHandle> uidToHandle_;
		uint32_t nextUid_ = 1;

		//--------- functions ----------------------------------------------------

		// ユニークな名前を作成
		std::string MakeUniqueName(const std::string& base);
		std::string MakeUniqueNameFromRequested(const std::string& requested, uint32_t exceptUid);
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Track/Asset/UIAnimationTrackAsset.h>
#include <Engine/Object/Data/Canvas/CanvasCommon.h>

// c++
#include <cstdint>
#include <string>
#include <vector>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationClip
	//	UIアニメーションクリップアセット、共有データ
	//============================================================================

	// アニメーションクリップ
	struct UIAnimationClip {
	
		// ファイルを保存する基底パス
		static inline const std::string kBaseJsonPath = "UIEditor/UIAnimation/";

		// 要素の名前
		uint32_t uid = 0;
		std::string name;

		// 描画キャンバス
		CanvasType canvasType = CanvasType::Sprite;

		// トラック定義リスト
		std::vector<UIAnimationTrackAsset> tracks;

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);
	};

	// UIアニメーションの登録情報
	struct UIAnimationEntry {

		// 読み込んだ元のファイルパス
		std::string filePath;
		UIAnimationClip clip;
	};
} // SakuEngine
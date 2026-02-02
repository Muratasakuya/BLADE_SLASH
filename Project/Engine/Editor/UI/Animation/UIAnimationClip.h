#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Canvas/CanvasCommon.h>
#include <Engine/Utility/Animation/ValueSource/AnimationChannel.h>

// c++
#include <cstdint>
#include <string>
#include <vector>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationClip
	//	UIアニメーションクリップアセット、共有データ
	//============================================================================

	// アニメーションさせる値の名前
	enum class UIAnimationDataType :
		uint32_t {

		Translation,
		Rotation,
		Scale,
		Color,
	};

	// アニメーショントラック定義
	struct UIAnimationTrackDefinition {

		// 処理の種類、適用方法
		UIAnimationDataType type = UIAnimationDataType::Translation;
		AnimationApplyMode applyMode = AnimationApplyMode::Absolute;

		// 補間方法
		std::string valueSourceType = "Lerp";
		Json valueSource;

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);
	};

	// アニメーションクリップ
	struct UIAnimationClip {
	
		// ファイルを保存する基底パス
		static inline const std::string kBaseJsonPath = "UIEditor/UIAnimation/";

		// 要素の名前
		uint32_t uid = 0;
		std::string name;
		// 補間対象の描画キャンバス
		CanvasType canvasType = CanvasType::Sprite;

		// トラック定義リスト
		std::vector<UIAnimationTrackDefinition> tracks;

		// 対象のデータタイプを所持しているか
		bool HasType(UIAnimationDataType type) const;

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
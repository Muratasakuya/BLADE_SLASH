#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/ValueSource/Interface/IUIValueSource.h>
#include <Engine/Editor/UI/Animation/UIAnimationTarget.h>
#include <Engine/Utility/Animation/ValueSource/AnimationChannel.h>
#include <Engine/MathLib/MathUtils.h>

namespace SakuEngine {

	//============================================================================
	//	UIAnimationTrackAsset structure
	//============================================================================

	// UIアニメーショントラックアセット
	struct UIAnimationTrackAsset {

		// 補間対象コンポーネント
		UIAnimationTargetType target = UIAnimationTargetType::ParentRectTransform;
		// 補間対象プロパティ
		UIAnimationProperty property = UIAnimationProperty::Translation;

		// 値ソースタイプ
		UIValueSourceType valueSourceType = UIValueSourceType::Lerp;
		// アニメーション適用モード
		AnimationApplyMode applyMode = AnimationApplyMode::Absolute;

		// 値ソース
		std::unique_ptr<IUIValueSourceAsset<Vector2>> valueVec2;
		std::unique_ptr<IUIValueSourceAsset<float>> valueFloat;
		std::unique_ptr<IUIValueSourceAsset<Color>> valueColor;

		// プロパティ変更時に値ソースを確保
		void EnsureValueSourceAllocated(bool forceRecreate = false);

		// エディター
		void ImGui(const char* label);

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);
	};
} // SakuEngine
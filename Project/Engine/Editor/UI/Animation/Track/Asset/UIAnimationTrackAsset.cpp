#include "UIAnimationTrackAsset.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/ValueSource/UILerpValueSource.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UIAnimationTrackAsset classMethods
//============================================================================

void UIAnimationTrackAsset::EnsureValueSourceAllocated() {

	// プロパティに応じて値ソースを確保
	switch (property) {
		//============================================================================
		//	Vector2系プロパティ
		//============================================================================
	case UIAnimationProperty::Translation:
	case UIAnimationProperty::Scale:

		// Vector2値ソースを確保
		if (!valueVec2) {

			valueVec2 = std::make_unique<UILerpValueSourceAsset<Vector2>>();
		}
		// 他の値ソースを解放
		valueFloat.reset();
		valueColor.reset();
		break;
		//============================================================================
		//	float系プロパティ
		//============================================================================
	case UIAnimationProperty::Rotation:

		// float値ソースを確保
		if (!valueFloat) {

			valueFloat = std::make_unique<UILerpValueSourceAsset<float>>();
		}
		// 他の値ソースを解放
		valueVec2.reset();
		valueColor.reset();
		break;
		//============================================================================
		//	Color系プロパティ
		//============================================================================
	case UIAnimationProperty::Color:

		// Color値ソースを確保
		if (!valueColor) {

			valueColor = std::make_unique<UILerpValueSourceAsset<Color>>();
		}
		// 他の値ソースを解放
		valueVec2.reset();
		valueFloat.reset();
		break;
	}
}

void UIAnimationTrackAsset::ImGui(const char* label) {

	ImGui::PushID(label);

	EnumAdapter<UIAnimationTargetType>::Combo("target", &target);
	EnumAdapter<AnimationApplyMode>::Combo("applyMode", &applyMode);

	if (EnumAdapter<UIAnimationProperty>::Combo("property", &property)) {
		// プロパティ変更時に値ソースを確保
		EnsureValueSourceAllocated();
	}

	// 値ソースの編集
	switch (property) {
	case UIAnimationProperty::Translation:
	case UIAnimationProperty::Scale:
		if (valueVec2) {

			valueVec2->ImGui("valueVec2");
		}
		break;
	case UIAnimationProperty::Rotation:
		if (valueFloat) {

			valueFloat->ImGui("valueFloat");
		}
		break;
	case UIAnimationProperty::Color:
		if (valueColor) {

			valueColor->ImGui("valueColor");
		}
		break;
	}
	ImGui::PopID();
}

void UIAnimationTrackAsset::FromJson(const Json& data) {

	target = EnumAdapter<UIAnimationTargetType>::FromString(data.value("target", "ParentRectTransform")).value();
	property = EnumAdapter<UIAnimationProperty>::FromString(data.value("property", "Translation")).value();
	applyMode = EnumAdapter<AnimationApplyMode>::FromString(data.value("applyMode", "Absolute")).value();

	// プロパティに応じて値ソースを確保
	EnsureValueSourceAllocated();

	// 値ソースの読み込み
	switch (property) {
	case UIAnimationProperty::Translation:
	case UIAnimationProperty::Scale:

		valueVec2->FromJson(data["valueVec2"]);
		break;
	case UIAnimationProperty::Rotation:
		
		valueFloat->FromJson(data["valueFloat"]);
		break;
	case UIAnimationProperty::Color:

		valueColor->FromJson(data["valueColor"]);
		break;
	}
}

void UIAnimationTrackAsset::ToJson(Json& data) {

	data["target"] = EnumAdapter<UIAnimationTargetType>::ToString(target);
	data["property"] = EnumAdapter<UIAnimationProperty>::ToString(property);
	data["applyMode"] = EnumAdapter<AnimationApplyMode>::ToString(applyMode);

	// 値ソースの書き出し
	switch (property) {
	case UIAnimationProperty::Translation:
	case UIAnimationProperty::Scale:
		if (valueVec2) {

			valueVec2->ToJson(data["valueVec2"]);
		}
		break;
	case UIAnimationProperty::Rotation:
		if (valueFloat) {

			valueFloat->ToJson(data["valueFloat"]);
		}
		break;
	case UIAnimationProperty::Color:
		if (valueColor) {

			valueColor->ToJson(data["valueColor"]);
		}
		break;
	}
}
#include "UIAnimationTrackAsset.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/ValueSource/Factory/UIValueSourceFactory.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UIAnimationTrackAsset classMethods
//============================================================================

void UIAnimationTrackAsset::EnsureValueSourceAllocated(bool forceRecreate) {

	// プロパティに応じて値ソースを確保
	switch (property) {
		//============================================================================
		//	Vector2系プロパティ
		//============================================================================
	case UIAnimationProperty::Translation:
	case UIAnimationProperty::Scale:

		// Vector2値ソースを確保
		if (forceRecreate || !valueVec2) {

			valueVec2 = UIValueSourceFactory::Create<Vector2>(valueSourceType);
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
		if (forceRecreate || !valueFloat) {

			valueFloat = UIValueSourceFactory::Create<float>(valueSourceType);
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
		if (forceRecreate || !valueColor) {

			valueColor = UIValueSourceFactory::Create<Color>(valueSourceType);
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
	// プロパティの編集
	if (EnumAdapter<UIAnimationProperty>::Combo("property", &property)) {

		// プロパティ変更時に値ソースを確保
		EnsureValueSourceAllocated(true);
	}
	// 値ソースタイプの編集
	if (EnumAdapter<UIValueSourceType>::Combo("valueSourceType", &valueSourceType)) {

		// 値ソースタイプ変更時に値ソースを確保
		EnsureValueSourceAllocated(true);
	}

	ImGui::Separator();

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
	valueSourceType = EnumAdapter<UIValueSourceType>::FromString(data.value("valueSourceType", "Lerp")).value();

	// プロパティに応じて値ソースを確保
	EnsureValueSourceAllocated(true);

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
	data["valueSourceType"] = EnumAdapter<UIValueSourceType>::ToString(valueSourceType);

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
#include "UIAnimationClip.h"

using namespace SakuEngine;

//============================================================================
//	UIAnimationClip classMethods
//============================================================================

void UIAnimationTrackDefinition::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	type = EnumAdapter<UIAnimationDataType>::FromString(data.value("type", "Translation")).value();
	applyMode = EnumAdapter<AnimationApplyMode>::FromString(data.value("applyMode", "Absolute")).value();

	valueSourceType = data.value("valueSourceType", "Lerp");
	valueSource = data.value("valueSource", Json{});
}

void UIAnimationTrackDefinition::ToJson(Json& data) {

	data["type"] = EnumAdapter<UIAnimationDataType>::ToString(type);
	data["applyMode"] = EnumAdapter<AnimationApplyMode>::ToString(applyMode);

	data["valueSourceType"] = valueSourceType;
	data["valueSource"] = valueSource;
}

bool UIAnimationClip::HasType(UIAnimationDataType type) const {

	for (const auto& track : tracks) {
		if (track.type == type) {
			return true;
		}
	}
	return false;
}

void UIAnimationClip::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	uid = data.value("uid", 0);
	name = data.value("name", "");
	canvasType = EnumAdapter<CanvasType>::FromString(data.value("canvasType", "Sprite")).value();

	tracks.clear();
	auto tracksJson = data.value("tracks", Json::array());
	for (const auto& trackJson : tracksJson) {

		UIAnimationTrackDefinition trackDef{};
		trackDef.FromJson(trackJson);
		tracks.emplace_back(trackDef);
	}
}

void UIAnimationClip::ToJson(Json& data) {

	data["version"] = 1;
	data["uid"] = uid;
	data["name"] = name;
	data["canvasType"] = EnumAdapter<CanvasType>::ToString(canvasType);

	Json array = Json::array();
	for (auto& track : tracks) {

		Json trackJson{};
		track.ToJson(trackJson);
		array.push_back(trackJson);
	}
	data["tracks"] = array;
}
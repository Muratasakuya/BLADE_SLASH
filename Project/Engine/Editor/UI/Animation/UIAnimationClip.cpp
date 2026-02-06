#include "UIAnimationClip.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UIAnimationClip classMethods
//============================================================================

void UIAnimationClip::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	uid = data.value("uid", 0);
	name = data.value("name", "");
	canvasType = EnumAdapter<CanvasType>::FromString(data.value("canvasType", "Sprite")).value();

	tracks.clear();
	for (const auto& trackData : data["tracks"]) {

		UIAnimationTrackAsset track{};
		track.FromJson(trackData);
		tracks.emplace_back(std::move(track));
	}
}

void UIAnimationClip::ToJson(Json& data) {

	data["version"] = 2;

	data["uid"] = uid;
	data["name"] = name;

	data["canvasType"] = EnumAdapter<CanvasType>::ToString(canvasType);

	Json array = Json::array();
	for (auto& track : tracks) {

		Json trackData{};
		track.ToJson(trackData);
		array.push_back(std::move(trackData));
	}
	data["tracks"] = std::move(array);
}
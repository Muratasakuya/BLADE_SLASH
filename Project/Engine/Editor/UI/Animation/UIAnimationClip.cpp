#include "UIAnimationClip.h"

using namespace SakuEngine;

//============================================================================
//	UIAnimationClip classMethods
//============================================================================

void UIAnimationClip::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	uid = data.value("uid", 0);
	name = data.value("name", "");
}

void UIAnimationClip::ToJson(Json& data) {

	data["version"] = 2;
	data["uid"] = uid;
	data["name"] = name;
}
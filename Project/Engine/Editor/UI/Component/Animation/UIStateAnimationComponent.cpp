#include "UIStateAnimationComponent.h"

using namespace SakuEngine;

//============================================================================
//	UIStateAnimationComponent classMethods
//============================================================================

UIStateAnimationComponent::UIStateAnimationComponent() {

	// 全て0で追加する
	stateToClipUid.emplace(UIElementState::Hidden, 0);
	stateToClipUid.emplace(UIElementState::ShowBegin, 0);
	stateToClipUid.emplace(UIElementState::ShowEnd, 0);
	stateToClipUid.emplace(UIElementState::Showing, 0);
	stateToClipUid.emplace(UIElementState::Focused, 0);
	stateToClipUid.emplace(UIElementState::Decided, 0);
}

void UIStateAnimationComponent::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	for (const auto& [stateJson, clipUid] : data["stateToClipUid"].items()) {

		UIElementState state = EnumAdapter<UIElementState>::FromString(stateJson).value();
		stateToClipUid[state] = clipUid.get<uint32_t>();
	}
}

void UIStateAnimationComponent::ToJson(Json& data) {

	Json stateToClipUidJson = Json::object();
	for (const auto& [state, clipUid] : stateToClipUid) {

		stateToClipUidJson[EnumAdapter<UIElementState>::ToString(state)] = clipUid;
	}
	data["stateToClipUid"] = stateToClipUidJson;
}
#include "UITextComponent.h"

using namespace SakuEngine;

//============================================================================
//	UITextComponent classMethods
//============================================================================

void UITextComponent::ImGui(const ImVec2& itemSize) {

	if (!text) {
		return;
	}
	text->ImGui(itemSize.x);
}

void UITextComponent::FromJson(const Json& data) {

	if (text) {
		text->FromJson(data);
		return;
	}

	// jsonデータをキャッシュ
	jsonCache_ = data;

	// オブジェクトIDをリセット
	objectId = 0;
	text = nullptr;
}

void UITextComponent::ToJson(Json& data) {

	// 存在する場合のみ
	if (text) {

		text->ToJson(data);
		return;
	}

	// ランタイムが無いならキャッシュを保存
	if (HasJsonCache()) {

		data = jsonCache_.value();
	}
}
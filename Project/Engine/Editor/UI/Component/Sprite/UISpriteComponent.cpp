#include "UISpriteComponent.h"

using namespace SakuEngine;

//============================================================================
//	UISpriteComponent classMethods
//============================================================================

void UISpriteComponent::ImGui(const ImVec2& itemSize) {

	if (!sprite) {
		return;
	}
	sprite->ImGui(itemSize.x);
}

void UISpriteComponent::FromJson(const Json& data) {

	if (sprite) {
		sprite->FromJson(data);
		return;
	}

	// jsonデータをキャッシュ
	jsonCache_ = data;

	// オブジェクトIDをリセット
	objectId = 0;
	sprite = nullptr;
}

void UISpriteComponent::ToJson(Json& data) {

	// 存在する場合のみ
	if (sprite) {

		sprite->ToJson(data);
		return;
	}

	// ランタイムが無いならキャッシュを保存
	if (HasJsonCache()) {

		data = jsonCache_.value();
	}
}
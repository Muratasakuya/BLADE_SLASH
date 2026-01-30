#include "UISpriteTransformComponent.h"

using namespace SakuEngine;

//============================================================================
//	UISpriteTransformComponent classMethods
//============================================================================

void UISpriteTransformComponent::ImGui(const ImVec2& itemSize) {

	if (!transform) {
		return;
	}
	transform->ImGui(itemSize.x);
}

void UISpriteTransformComponent::FromJson(const Json& data) {

	if (transform) {
		transform->FromJson(data);
		return;
	}

	// jsonデータをキャッシュ
	jsonCache_ = data;

	// オブジェクトをリセット
	transform = nullptr;
}

void UISpriteTransformComponent::ToJson(Json& data) {

	// 存在する場合のみ
	if (transform) {

		transform->ToJson(data);
		return;
	}

	// ランタイムが無いならキャッシュを保存
	if (HasJsonCache()) {

		data = jsonCache_.value();
	}
}
#include "UISpriteMaterialComponent.h"

using namespace SakuEngine;

//============================================================================
//	UISpriteMaterialComponent classMethods
//============================================================================

void UISpriteMaterialComponent::ImGui(const ImVec2& itemSize) {

	if (!material) {
		return;
	}
	material->ImGui(itemSize.x);
}

void UISpriteMaterialComponent::FromJson(const Json& data) {

	if (material) {
		material->FromJson(data);
		return;
	}

	// jsonデータをキャッシュ
	jsonCache_ = data;

	// オブジェクトをリセット
	material = nullptr;
}

void UISpriteMaterialComponent::ToJson(Json& data) {

	// 存在する場合のみ
	if (material) {

		material->ToJson(data);
		return;
	}

	// ランタイムが無いならキャッシュを保存
	if (HasJsonCache()) {

		data = jsonCache_.value();
	}
}
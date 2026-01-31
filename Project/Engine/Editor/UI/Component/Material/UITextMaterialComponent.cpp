#include "UITextMaterialComponent.h"

using namespace SakuEngine;

//============================================================================
//	UITextMaterialComponent classMethods
//============================================================================

void UITextMaterialComponent::ImGui(const ImVec2& itemSize) {

	if (!material) {
		return;
	}
	material->ImGui(itemSize.x);
}

void UITextMaterialComponent::FromJson(const Json& data) {

	if (material) {
		material->FromJson(data);
		return;
	}

	// jsonデータをキャッシュ
	jsonCache_ = data;

	// オブジェクトをリセット
	material = nullptr;
}

void UITextMaterialComponent::ToJson(Json& data) {

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
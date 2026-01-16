#include "UISlateWidget.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	UISlateWidget classMethods
//============================================================================

bool UIRect::Contains(const Vector2& checkPos) const {

	// 矩形内判定
	bool result = (pos.x <= checkPos.x && pos.y <= checkPos.y &&
		checkPos.x <= pos.x + size.x && checkPos.y <= pos.y + size.y);
	return result;
}

UIRect UISlateWidget::ComputeRectFromAnchorData(const UIRect& parent, const UIAnchorData& data) {

	// 親矩形サイズ
	Vector2 pSize = parent.size;

	// ストレッチ判定
	bool stretchX = (data.anchors.min.x != data.anchors.max.x);
	bool stretchY = (data.anchors.min.y != data.anchors.max.y);

	// アンカー位置計算
	float left = parent.pos.x + data.anchors.min.x * pSize.x;
	float top = parent.pos.y + data.anchors.min.y * pSize.y;
	float right = parent.pos.x + data.anchors.max.x * pSize.x;
	float bottom = parent.pos.y + data.anchors.max.y * pSize.y;

	// 矩形計算
	UIRect rect{};
	// 
	if (!stretchX) {

		rect.pos.x = left + data.offsets.left;
		rect.size.x = data.offsets.right;
	} else {

		rect.pos.x = left + data.offsets.left;
		rect.size.x = (right - data.offsets.right) - rect.pos.x;
	}
	// 
	if (!stretchY) {

		rect.pos.y = top + data.offsets.top;
		rect.size.y = data.offsets.bottom;
	} else {

		rect.pos.y = top + data.offsets.top;
		rect.size.y = (bottom - data.offsets.bottom) - rect.pos.y;
	}
	return rect;
}

void UISlateWidget::FromJson(const Json& data) {

	id_ = data.value("id", id_);
	generation_ = data.value("generation", generation_);
	name_ = data.value("name", name_);
	visibility_ = EnumAdapter<UIVisibility>::FromString(data.value("visibility", "Visible")).value();
	enabled_ = data.value("enabled", true);

	// レイアウト
	if (data.contains("layout")) {

		const auto& layout = data["layout"];
		layout_.anchors.min = Vector2::FromJson(layout["anchors"]["min"]);
		layout_.anchors.max = Vector2::FromJson(layout["anchors"]["max"]);
		layout_.offsets.left = layout["offsets"].value("l", 0.0f);
		layout_.offsets.top = layout["offsets"].value("t", 0.0f);
		layout_.offsets.right = layout["offsets"].value("r", 0.0f);
		layout_.offsets.bottom = layout["offsets"].value("b", 0.0f);
		layout_.alignment = Vector2::FromJson(layout["alignment"]);
	}
}

void UISlateWidget::ToJson(Json& data) {

	data["id"] = id_;
	data["generation"] = generation_;
	data["name"] = name_;
	data["visibility"] = EnumAdapter<UIVisibility>::ToString(visibility_);
	data["enabled"] = enabled_;

	// レイアウト
	Json layout;
	layout["anchors"]["min"] = layout_.anchors.min.ToJson();
	layout["anchors"]["max"] = layout_.anchors.max.ToJson();
	layout["offsets"]["l"] = layout_.offsets.left;
	layout["offsets"]["t"] = layout_.offsets.top;
	layout["offsets"]["r"] = layout_.offsets.right;
	layout["offsets"]["b"] = layout_.offsets.bottom;
	layout["alignment"] = layout_.alignment.ToJson();
	data["layout"] = layout;
}
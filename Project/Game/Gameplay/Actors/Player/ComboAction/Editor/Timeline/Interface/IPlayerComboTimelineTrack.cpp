#include "IPlayerComboTimelineTrack.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>

//============================================================================
//	IPlayerComboTimelineTrack classMethods
//============================================================================

float PlayerComboTimelineHelper::SnapTime(float t) {

	float step = 0.1f;
	if (step <= 0.0f) {
		return t;
	}
	float n = std::round(t / step);
	return n * step;
}

float PlayerComboTimelineHelper::TimeToLocalX(
	const PlayerComboTimelineView& view, float time) {

	return time * view.pixelsPerSecond;
}

float PlayerComboTimelineHelper::LocalXToTime(
	const PlayerComboTimelineView& view, float localX) {

	// ゼロ除算防止
	if (view.pixelsPerSecond <= 0.0f) {
		return 0.0f;
	}
	// 時間に変換
	return localX / view.pixelsPerSecond;
}

void PlayerComboTimelineHelper::DrawTrackGridLines(const PlayerComboTimelineDrawContext& context, float trackTopY) {

	float y0 = trackTopY;
	float y1 = trackTopY + context.view.trackHeight;

	float grid = context.view.gridStep;
	if (grid <= 0.0f) {
		grid = 0.1f;
	}

	float major = context.view.majorGridStep;
	if (major <= 0.0f) {
		major = 1.0f;
	}

	// 表示範囲
	int32_t iStart = static_cast<int32_t>(std::floor(context.visibleTimeStart / grid));
	int32_t iEnd = static_cast<int32_t>(std::ceil(context.visibleTimeEnd / grid));

	// タイムライン領域にだけ描く
	float clipL = context.contentScreenPos.x;
	float clipR = context.contentScreenPos.x + context.contentScreenSize.x;

	context.drawList->PushClipRect(ImVec2(clipL, y0), ImVec2(clipR, y1), true);

	for (int32_t i = iStart; i <= iEnd; ++i) {

		float t = static_cast<float>(i) * grid;

		float localX = context.view.contentPadding.x +
			PlayerComboTimelineHelper::TimeToLocalX(context.view, t) -
			context.scroll.x;

		float x = context.contentScreenPos.x + localX;

		// major判定
		float majorIndexF = std::round(t / major);
		float majorTime = majorIndexF * major;
		bool isMajor = (std::fabs(t - majorTime) <= (grid * 0.25f + Config::kEpsilon));

		// rulerより少し暗め
		ImU32 color = isMajor ? IM_COL32(65, 65, 65, 255) : IM_COL32(35, 35, 35, 255);
		context.drawList->AddLine(ImVec2(x, y0), ImVec2(x, y1), color);
	}
	context.drawList->PopClipRect();
}
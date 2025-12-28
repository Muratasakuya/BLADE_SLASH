#include "IPlayerComboTimelineTrack.h"

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
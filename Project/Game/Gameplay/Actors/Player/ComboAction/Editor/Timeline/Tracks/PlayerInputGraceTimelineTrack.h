#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/Timeline/Interface/IPlayerComboTimelineTrack.h>

//============================================================================
//	PlayerInputGraceTimelineTrack class
//	プレイヤーの入力猶予時間を表示するタイムライントラック
//============================================================================
class PlayerInputGraceTimelineTrack :
	public IPlayerComboTimelineTrack {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerInputGraceTimelineTrack() = default;
	~PlayerInputGraceTimelineTrack() = default;

	void DrawTrack(PlayerComboTimelineDrawContext& context, float trackTopY) override;

	//--------- accessor -----------------------------------------------------

	const char* TrackName() const override { return "InputGrace"; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 今編集しているステップID
	uint32_t editStepId_ = 0;

	// 各デバイスの入力キャプチャ状態
	bool isCaptureKeyboard_ = false;
	bool isCapturePad_ = false;

	// ポップアップメニュー関連
	uint32_t popupCandidateStepId_ = 0;
	bool popupCandidateDragged_ = false;
};
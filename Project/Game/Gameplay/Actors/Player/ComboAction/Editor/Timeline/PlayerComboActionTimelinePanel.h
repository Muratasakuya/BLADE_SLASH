#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/Timeline/Interface/IPlayerComboTimelineTrack.h>

//============================================================================
//	PlayerComboActionTimelinePanel class
//	プレイヤーコンボアクションの調整、作成を行うタイムラインパネル
//============================================================================
class PlayerComboActionTimelinePanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboActionTimelinePanel();
	~PlayerComboActionTimelinePanel() = default;

	// タイムライン描画
	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select);

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// タイムライン、表示設定
	PlayerComboTimelineRuntime runtime_;
	PlayerComboTimelineView view_;

	// track list
	std::vector<std::unique_ptr<IPlayerComboTimelineTrack>> tracks_;

	//--------- functions ----------------------------------------------------

	// runtime表示
	void DrawRuntime(PlayerComboTimelineDrawContext& context);
	// ルーラー、スクラブ
	void DrawRuler(PlayerComboTimelineDrawContext& context);

	// トラック名領域
	void DrawTrackNames(const std::vector<std::unique_ptr<IPlayerComboTimelineTrack>>& tracks, float totalTrackHeight, float syncedScrollY);

	// content領域
	void DrawTracks(PlayerComboTimelineDrawContext& context, const std::vector<std::unique_ptr<IPlayerComboTimelineTrack>>& tracks);

	// 入力
	void HandleKeyStep(PlayerComboTimelineDrawContext& context);
};
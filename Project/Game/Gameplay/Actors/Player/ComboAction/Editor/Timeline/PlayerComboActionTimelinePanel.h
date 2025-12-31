#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/Timeline/Interface/IPlayerComboTimelineTrack.h>

// front
class PlayerComboExecutor;

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

	// Executorをセット
	void SetExecutor(PlayerComboExecutor* executor) { executor_ = executor; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// タイムライン、表示設定
	PlayerComboTimelineRuntime runtime_;
	PlayerComboTimelineView view_;

	// タイムライントラックリスト
	std::vector<std::unique_ptr<IPlayerComboTimelineTrack>> tracks_;

	// エディター
	bool isEditParam_ = false;

	// 実行用
	PlayerComboExecutor* executor_;

	//--------- functions ----------------------------------------------------

	// runtime表示
	void DrawRuntime(PlayerComboTimelineDrawContext& context);
	// ルーラー、スクラブ
	void DrawRuler(PlayerComboTimelineDrawContext& context);

	// トラック名領域
	void DrawTrackNames(const std::vector<std::unique_ptr<IPlayerComboTimelineTrack>>& tracks, float syncedScrollY);

	// content領域
	void DrawTracks(PlayerComboTimelineDrawContext& context, const std::vector<std::unique_ptr<IPlayerComboTimelineTrack>>& tracks);

	// 入力
	void HandleKeyStep(PlayerComboTimelineDrawContext& context);

	// エディターパラメータ調整
	void EditParam();

	// Executorがタイムライン制御を使っているか
	bool IsUsingExecutorTimeline() const;
};
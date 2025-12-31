#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/Timeline/PlayerComboActionTimelinePanel.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Interface/IPlayerComboActionEditorPanel.h>

//============================================================================
//	PlayerComboTimelinePanel class
//	プレイヤーコンボアクションタイムラインパネル
//============================================================================
class PlayerComboTimelinePanel :
	public IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboTimelinePanel() = default;
	~PlayerComboTimelinePanel() = default;

	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) override;

	//--------- accessor -----------------------------------------------------

	const char* PanelName() const override { return "Timeline"; }

	void SetExecutor(PlayerComboExecutor* executor) { timeline_.SetExecutor(executor); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// タイムラインパネル
	PlayerComboActionTimelinePanel timeline_;
};
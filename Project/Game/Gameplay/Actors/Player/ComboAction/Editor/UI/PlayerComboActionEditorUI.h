#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerComboActionModel.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Interface/IPlayerComboActionEditorPanel.h>

//============================================================================
//	PlayerComboActionEditorUI class
//	プレイヤーコンボアクションエディターUI
//============================================================================
class PlayerComboActionEditorUI {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboActionEditorUI();
	~PlayerComboActionEditorUI() = default;

	// 全てのパネルを描画
	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// パネル
	std::vector<std::unique_ptr<IPlayerComboActionEditorPanel>> panels_;

	// タイムライン、別で描画
	std::unique_ptr<IPlayerComboActionEditorPanel> timelinePanel_;
};
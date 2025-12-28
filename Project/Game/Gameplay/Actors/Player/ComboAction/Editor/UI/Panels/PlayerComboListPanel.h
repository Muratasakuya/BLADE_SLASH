#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Interface/IPlayerComboActionEditorPanel.h>

//============================================================================
//	PlayerComboListPanel class
//	プレイヤーコンボアクションリストパネル
//============================================================================
class PlayerComboListPanel :
	public IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboListPanel() = default;
	~PlayerComboListPanel() = default;

	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) override;

	//--------- accessor -----------------------------------------------------

	const char* PanelName() const override { return "Actions"; }
};
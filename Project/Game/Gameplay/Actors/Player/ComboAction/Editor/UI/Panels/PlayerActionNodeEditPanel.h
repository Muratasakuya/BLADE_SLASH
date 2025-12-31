#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Interface/IPlayerComboActionEditorPanel.h>

//============================================================================
//	PlayerActionNodeEditPanel class
//	プレイヤーのアクションノード編集パネル
//============================================================================
class PlayerActionNodeEditPanel :
	public IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerActionNodeEditPanel() = default;
	~PlayerActionNodeEditPanel() = default;

	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) override;

	//--------- accessor -----------------------------------------------------

	const char* PanelName() const override { return "Action Edit"; }
};
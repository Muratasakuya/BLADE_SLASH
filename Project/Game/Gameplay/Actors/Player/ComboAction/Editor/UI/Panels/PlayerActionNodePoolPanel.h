#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Interface/IPlayerComboActionEditorPanel.h>

//============================================================================
//	PlayerActionNodePoolPanel class
//	プレイヤーアクションノードプールパネル
//============================================================================
class PlayerActionNodePoolPanel :
	public IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerActionNodePoolPanel() = default;
	~PlayerActionNodePoolPanel() = default;

	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) override;

	//--------- accessor -----------------------------------------------------

	const char* PanelName() const override { return "Action Nodes"; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 作成するノードタイプ
	PlayerActionNodeType createType_;
};
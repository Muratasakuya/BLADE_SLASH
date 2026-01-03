#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/Interface/IPlayerComboActionEditorPanel.h>

//============================================================================
//	PlayerGuardConditionPanel class
//	プレイヤーガード条件パネル
//============================================================================
class PlayerGuardConditionPanel :
	public IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerGuardConditionPanel() = default;
	~PlayerGuardConditionPanel() = default;

	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) override;

	//--------- accessor -----------------------------------------------------

	const char* PanelName() const override { return "GuardCondition"; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 追加する開始条件タイプ
	PlayerGuardConditionType newStartConditionType_ = static_cast<PlayerGuardConditionType>(0);
	// 選択中の開始条件インデックス
	int32_t selectedStartConditionIndex_ = -1;
	// 最後に描画したコンボインデックス
	int32_t lastComboIndex_ = -1;
};
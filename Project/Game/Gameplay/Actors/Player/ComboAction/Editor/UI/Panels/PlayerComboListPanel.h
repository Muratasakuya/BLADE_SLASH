#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/ImGuiHelper.h>
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
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 保存モーダル状態
	SakuEngine::JsonSaveState saveAllState_{};
	SakuEngine::JsonSaveState saveNodeState_{};

	// ステータス表示
	std::string status_;
};
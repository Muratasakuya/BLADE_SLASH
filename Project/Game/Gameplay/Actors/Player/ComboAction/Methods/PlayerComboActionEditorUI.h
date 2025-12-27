#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerComboActionModel.h>

//============================================================================
//	IPlayerComboActionEditorPanel class
//	プレイヤーコンボアクションエディターパネルインターフェース
//============================================================================
class IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPlayerComboActionEditorPanel() = default;
	virtual ~IPlayerComboActionEditorPanel() = default;

	// エディター
	virtual void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) = 0;

	//--------- accessor -----------------------------------------------------

	// パネル名取得
	virtual const char* PanelName() const = 0;
};

//============================================================================
// IPlayerComboActionEditorPanelを継承した各パネルクラス
//============================================================================
class ActionNodePoolPanel final :
	public IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

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
class ComboListPanel final :
	public IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) override;

	//--------- accessor ----------------------------------------------------

	const char* PanelName() const override { return "Actions (combos)"; }
};
class ComboDetailPanel final :
	public IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) override;

	//--------- accessor ----------------------------------------------------

	const char* PanelName() const override { return "Action Detail"; }
};

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

	// 各パネル
	// アクションノード
	std::unique_ptr<ActionNodePoolPanel> nodePool_;
	// コンボリスト
	std::unique_ptr<ComboListPanel> comboList_;
	// コンボ詳細
	std::unique_ptr<ComboDetailPanel> comboDetail_;
};
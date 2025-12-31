#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/UI/PlayerComboActionEditorUI.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerReactionAreaChecker.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Runtime/Execute/PlayerComboExecutor.h>

//============================================================================
//	PlayerComboActionEditor class
//	プレイヤーのコンボアクションを作成するエディター
//============================================================================
class PlayerComboActionEditor :
	public SakuEngine::IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboActionEditor() :SakuEngine::IGameEditor("PlayerComboAction") {}
	~PlayerComboActionEditor() = default;

	// 初期化
	void Init(Player* player);

	// 更新
	void Update();

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 攻撃対象の設定
	void SetAttackTarget(const SakuEngine::GameObject3D* target);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// アクション情報
	PlayerComboActionModel model_;
	// リアクションエリアチェッカー
	std::unique_ptr<PlayerReactionAreaChecker> areaChecker_;
	// コンボ実行
	std::unique_ptr<PlayerComboExecutor> executor_;
	// エディターUI
	PlayerComboActionEditorSelection selection_;
	PlayerComboActionEditorUI editorUI_;

	//--------- functions ----------------------------------------------------

};
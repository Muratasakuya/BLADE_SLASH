#include "PlayerComboActionEditor.h"

//============================================================================
//	PlayerComboActionEditor classMethods
//============================================================================

void PlayerComboActionEditor::SetAttackTarget(const SakuEngine::GameObject3D* target) {

	// 攻撃対象を設定
	areaChecker_->SetAttackTarget(target);
	model_.SetAttackTarget(target);
	executor_->SetAttackTarget(target);
}

void PlayerComboActionEditor::Init(Player* player) {

	// リアクションエリアチェッカー初期化
	areaChecker_ = std::make_unique<PlayerReactionAreaChecker>();
	areaChecker_->Init();
	areaChecker_->SetPlayer(player);
	// コンボ実行初期化
	executor_ = std::make_unique<PlayerComboExecutor>();
	executor_->Init(player, &model_, areaChecker_.get());

	// モデルにプレイヤーとエリアチェッカーを設定
	model_.SetPlayer(player);
	model_.SetAreaChecker(areaChecker_.get());
	// エディターUIにExecutorを設定
	editorUI_.SetExecutor(executor_.get());
}

void PlayerComboActionEditor::Update() {

	// リアクションエリアチェッカー更新
	areaChecker_->Update();
	// コンボ実行更新
	executor_->Update();
}

void PlayerComboActionEditor::ImGui() {

	if (ImGui::BeginTabBar("Hierarchy")) {

		if (ImGui::BeginTabItem("ComboAction")) {

			editorUI_.Draw(model_, selection_);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Area")) {

			areaChecker_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
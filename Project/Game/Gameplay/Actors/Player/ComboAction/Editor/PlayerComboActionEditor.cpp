#include "PlayerComboActionEditor.h"

//============================================================================
//	PlayerComboActionEditor classMethods
//============================================================================

void PlayerComboActionEditor::SetAttackTarget(const SakuEngine::GameObject3D* target) {

	// 攻撃対象を設定
	areaChecker_->SetAttackTarget(target);
	model_.SetAttackTarget(target);
}

void PlayerComboActionEditor::Init(Player* player) {

	// リアクションエリアチェッカー初期化
	areaChecker_ = std::make_unique<PlayerReactionAreaChecker>();
	areaChecker_->Init();
	areaChecker_->SetPlayer(player);

	// モデルにプレイヤーとエリアチェッカーを設定
	model_.SetPlayer(player);
	model_.SetAreaChecker(areaChecker_.get());
}

void PlayerComboActionEditor::Update() {

	// リアクションエリアチェッカー更新
	areaChecker_->Update();
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
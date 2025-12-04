#include "BossEnemyGreatAttackApproach.h"

//============================================================================
//	BossEnemyGreatAttackApproach classMethods
//============================================================================

BossEnemyGreatAttackApproach::BossEnemyGreatAttackApproach() {

	// 初期化
	movePendulum_.Init();
}

void BossEnemyGreatAttackApproach::Enter() {

	canExit_ = true;
}

void BossEnemyGreatAttackApproach::Update() {


}

void BossEnemyGreatAttackApproach::Exit() {


}

void BossEnemyGreatAttackApproach::ImGui() {

	movePendulum_.ImGui();
}

void BossEnemyGreatAttackApproach::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	movePendulum_.FromJson(data.value("MovePendulum", Json()));
}

void BossEnemyGreatAttackApproach::SaveJson(Json& data) {

	movePendulum_.ToJson(data["MovePendulum"]);
}
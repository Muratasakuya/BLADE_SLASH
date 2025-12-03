#include "BossEnemyGreatAttackExecute.h"

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyGreatAttackExecute classMethods
//============================================================================

BossEnemyGreatAttackExecute::BossEnemyGreatAttackExecute() {

	// 初期化値
	canExit_ = false;
}

void BossEnemyGreatAttackExecute::Enter() {

	// チャージ終了アニメーション再生
	bossEnemy_->SetNextAnimation("bossEnemy_endChargeGreatAttack", false, nextAnimDuration_);
}

void BossEnemyGreatAttackExecute::Update() {

	// 時間を更新
	nextTimer_.Update();

	// 時間経過後次の状態に進む
	if (nextTimer_.IsReached()) {

		canExit_ = true;
	}
}

void BossEnemyGreatAttackExecute::Exit() {

	// リセット
	nextTimer_.Reset();
	canExit_ = false;
}

void BossEnemyGreatAttackExecute::ImGui() {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::Separator();

	nextTimer_.ImGui("NextTimer", false);
}

void BossEnemyGreatAttackExecute::ApplyJson(const Json& data) {

	nextTimer_.FromJson(data.value("NextTimer", Json()));
	nextAnimDuration_ = data.value("nextAnimDuration_", 0.0f);
}

void BossEnemyGreatAttackExecute::SaveJson(Json& data) {

	nextTimer_.ToJson(data["NextTimer"]);
	data["nextAnimDuration_"] = nextAnimDuration_;
}
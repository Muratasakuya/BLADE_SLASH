#include "BossEnemyGreatAttackApproach.h"

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyGreatAttackApproach classMethods
//============================================================================

BossEnemyGreatAttackApproach::BossEnemyGreatAttackApproach() {

	// 初期化
	movePendulum_.Init();
}

void BossEnemyGreatAttackApproach::Enter() {

	// 状態初期化
	currentState_ = State::Approach;
	canExit_ = false;
	movePendulum_.Reset();

	// 最初の補間位置までの座標を設定する
	startMoveAnim_.SetStart(bossEnemy_->GetTranslation());
	startMoveAnim_.Start();
}

void BossEnemyGreatAttackApproach::Update() {

	// 状態毎の更新
	switch (currentState_) {
	case BossEnemyGreatAttackApproach::State::Approach:

		UpdateApproach();
		break;
	case BossEnemyGreatAttackApproach::State::Attack:

		UpdateAttack();
		break;
	}
}

void BossEnemyGreatAttackApproach::UpdateApproach() {

	// 補間先座標は常に更新する
	startMoveAnim_.SetEnd(movePendulum_.GetMinPos());

	// 座標補間
	Vector3 newPos = bossEnemy_->GetTranslation();
	startMoveAnim_.LerpValue(newPos);
	bossEnemy_->SetTranslation(newPos);

	// 終了次第攻撃に移る
	if (startMoveAnim_.IsFinished()) {

		currentState_ = State::Attack;
	}
}

void BossEnemyGreatAttackApproach::UpdateAttack() {

	// 振り子を更新して振り子位置をセットする
	movePendulum_.Update();
	bossEnemy_->SetTranslation(movePendulum_.currentPos);

	// 最大到達回数に達したら終了
	if (pendulumMaxReachCount_ <= movePendulum_.reachCount) {

		canExit_ = true;
	}
}

void BossEnemyGreatAttackApproach::UpdateAlways() {

	// 位置、回転を更新する
	Vector3 forward = followCamera_->GetTransform().GetForward();
	Quaternion cameraRotation = Quaternion::LookRotation(forward.Normalize(), Vector3(0.0f, 1.0f, 0.0f));
	cameraRotation = Quaternion::Normalize(cameraRotation);
	// カメラのXZ回転は0.0fにしてYの回転のみ反映させる
	movePendulum_.rotation = cameraRotation;
	movePendulum_.anchor = player_->GetTranslation() + cameraRotation * pendulumOffset_;
}

void BossEnemyGreatAttackApproach::Exit() {


}

void BossEnemyGreatAttackApproach::ImGui() {

	ImGui::DragFloat3("PendulumOffset", &pendulumOffset_.x, 0.01f);
	int32_t reachCount = static_cast<int32_t>(pendulumMaxReachCount_);
	ImGui::DragInt("PendulumMaxReachCount", &reachCount, 1, 0);
	pendulumMaxReachCount_ = static_cast<uint32_t>(reachCount);

	if (ImGui::CollapsingHeader("MovePendulum")) {

		movePendulum_.ImGui();
	}
	if (ImGui::CollapsingHeader("StartMoveAnim")) {

		startMoveAnim_.ImGui("StartMoveAnim", false);
	}
}

void BossEnemyGreatAttackApproach::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	movePendulum_.FromJson(data.value("MovePendulum", Json()));
	pendulumOffset_ = Vector3::FromJson(data.value("PendulumOffset", Json()));
	pendulumMaxReachCount_ = data.value("PendulumMaxReachCount", 3u);
	startMoveAnim_.FromJson(data.value("StartMoveAnim", Json()));
}

void BossEnemyGreatAttackApproach::SaveJson(Json& data) {

	movePendulum_.ToJson(data["MovePendulum"]);
	data["PendulumOffset"] = pendulumOffset_.ToJson();
	data["PendulumMaxReachCount"] = pendulumMaxReachCount_;
	startMoveAnim_.ToJson(data["StartMoveAnim"]);
}
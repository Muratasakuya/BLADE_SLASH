#include "BossEnemyStunState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyStunState classMethods
//============================================================================

void BossEnemyStunState::CreateEffect() {

	// animationの時間からstunから復帰する時間を取得する
	beginStunAnimationTime_ = bossEnemy_->GetAnimationDuration("bossEnemy_stun");
	updateStunAnimationTime_ = bossEnemy_->GetAnimationDuration("bossEnemy_stunUpdate");

	toughnessDecreaseTimer_ = 0.0f;
	canExit_ = false;
}

void BossEnemyStunState::Enter() {

	// スタン状態を開始する
	bossEnemy_->SetNextAnimation("bossEnemy_stun", false, nextAnimDuration_);

	// α値が透明になっているときがあるので1.0fに戻しておく
	bossEnemy_->SetAlpha(1.0f);

	// 最初の状態を設定
	currentState_ = State::Begin;
}

void BossEnemyStunState::Update() {

	// 状態別で更新
	switch (currentState_) {
	case BossEnemyStunState::State::Begin: {

		// 最初のanimationが終了したら次の状態に遷移
		if (bossEnemy_->IsAnimationFinished()) {

			// スタン状態の更新
			bossEnemy_->SetNextAnimation("bossEnemy_stunUpdate", true, nextAnimDuration_);

			currentState_ = State::Update;
		}
		break;
	}
	case BossEnemyStunState::State::Update: {

		// 指定回数animationがループしたら終了
		if (maxAnimationCount_ < bossEnemy_->GetAnimationRepeatCount()) {

			// 遷移可能状態にする
			canExit_ = true;
		}

		// stun状態の経過時間を進める
		toughnessDecreaseTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
		// 経過時間を設定する
		bossEnemy_->SetDecreaseToughnessProgress(toughnessDecreaseTimer_ / toughnessDecreaseTime_);
		break;
	}
	}
}

void BossEnemyStunState::Exit() {

	// リセット
	bossEnemy_->ResetAnimation();
	canExit_ = false;
	toughnessDecreaseTimer_ = 0.0f;
}

void BossEnemyStunState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragInt("maxAnimationCount", &maxAnimationCount_, 1);

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());

	// 経過率
	float progress = toughnessDecreaseTimer_ / toughnessDecreaseTime_;
	ImGui::Text("toughnessDecreaseTimer: %f", toughnessDecreaseTimer_);
	ImGui::Text("toughnessDecreaseTime: %f", toughnessDecreaseTime_);
	ImGui::ProgressBar(progress, ImVec2(256.0f, 0.0f));

	ImGui::Text("animationRepeatCount: %d", bossEnemy_->GetAnimationRepeatCount());
}

void BossEnemyStunState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	maxAnimationCount_ = SakuEngine::JsonAdapter::GetValue<int>(data, "maxAnimationCount_");

	// animation再生時間の合計
	// stunUpdateAnimation再生回数 + 最初のstunAnimation再生回数
	toughnessDecreaseTime_ = (beginStunAnimationTime_ +
		updateStunAnimationTime_ * static_cast<float>(maxAnimationCount_)) -
		nextAnimDuration_ * static_cast<float>(maxAnimationCount_ + 1);
}

void BossEnemyStunState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["maxAnimationCount_"] = maxAnimationCount_;
}
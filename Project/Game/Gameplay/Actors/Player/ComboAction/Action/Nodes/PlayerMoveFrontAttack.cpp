#include "PlayerMoveFrontAttack.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerMoveFrontAttack classMethods
//============================================================================

void PlayerMoveFrontAttack::SetProgress([[maybe_unused]] float progress) {

	// ここは時間がかかるのでまだ実装しない。
	// とりあえず入力を完璧に実装する
}

bool PlayerMoveFrontAttack::IsFinished() const {

	return exitTimer_.IsReached();
}

float PlayerMoveFrontAttack::GetTotalTime() const {

	// 終了条件に基づいた合計時間を返す
	float totalTime = 0.0f;
	switch (endCondition_) {
	case PlayerMoveFrontAttack::EndCondition::EndAnim:

		// アニメーションの合計時間
		totalTime = player_->GetAnimationDuration(animationName_) + nextAnimDuration_;
		break;
	case PlayerMoveFrontAttack::EndCondition::LerpPos:

		totalTime = lerpPos_.timer.target_;
		break;
	case PlayerMoveFrontAttack::EndCondition::LerpRotate:

		totalTime = lerpRotate_.timer.target_;
		break;
	}
	// ExitTimeを加算
	totalTime += exitTimer_.target_;
	return totalTime;
}

bool PlayerMoveFrontAttack::IsEndConditionReached() const {

	// 終了条件に基づいて時間が過ぎたかを返す
	bool isReached = false;
	switch (endCondition_) {
	case PlayerMoveFrontAttack::EndCondition::EndAnim:

		isReached = player_->IsAnimationFinished();
		break;
	case PlayerMoveFrontAttack::EndCondition::LerpPos:

		isReached = lerpPos_.timer.IsReached();
		break;
	case PlayerMoveFrontAttack::EndCondition::LerpRotate:

		isReached = lerpRotate_.timer.IsReached();
		break;
	}
	return isReached;
}

void PlayerMoveFrontAttack::Enter() {

	// タイマーリセット
	lerpPos_.timer.Reset();
	lerpRotate_.timer.Reset();
	exitTimer_.Reset();

	// アニメーション再生
	player_->SetNextAnimation(animationName_, false, nextAnimDuration_);

	//============================================================================
	// 移動開始位置、目標位置の設定
	//============================================================================
	{
		lerpPos_.start = player_->GetTranslation();
		Vector3 targetPos = attackTarget_->GetTranslation();
		// 攻撃対象が範囲内にいる場合は攻撃対象に向かわせる
		if (areaChecker_->IsInRange(AreaReactionType::LerpPos)) {

			// 敵への方向を取得
			Vector3 direction = Math::GetDirection3D(*player_, *attackTarget_);
			lerpPos_.target = targetPos - direction * distance_;
		}
		// 範囲外の場合は正面方向に移動させる
		else {

			lerpPos_.target = lerpPos_.start + player_->GetTransform().GetForward() * distance_;
		}
	}
	//============================================================================
	// 回転開始、目標回転の設定
	//============================================================================
	{
		lerpRotate_.start = player_->GetRotation();
		// 攻撃対象が範囲内にいる場合は攻撃対象方向を向かせる
		if (areaChecker_->IsInRange(AreaReactionType::LerpRotate)) {

			Vector3 direction = Math::GetDirection3D(*player_, *attackTarget_);
			lerpRotate_.target = Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up));
		}
		// 範囲外の場合はそのまま
		else {

			lerpRotate_.target = player_->GetRotation();
		}
	}
}

void PlayerMoveFrontAttack::Update() {

	// 時間経過で処理終了
	if (IsEndConditionReached()) {

		exitTimer_.Update();
		return;
	}

	// 座標補間更新
	{
		lerpPos_.timer.Update();
		Vector3 pos = Vector3::Lerp(lerpPos_.start, lerpPos_.target, lerpPos_.timer.easedT_);
		player_->SetTranslation(pos);
	}
	// 回転補間更新
	{
		lerpRotate_.timer.Update();
		Quaternion rotate = Quaternion::Slerp(lerpRotate_.start, lerpRotate_.target, lerpRotate_.timer.easedT_);
		player_->SetRotation(Quaternion::Normalize(rotate));
	}
}

void PlayerMoveFrontAttack::Exit() {

	// タイマーリセット
	lerpPos_.timer.Reset();
	lerpRotate_.timer.Reset();
	exitTimer_.Reset();
}

void PlayerMoveFrontAttack::ImGui() {

	ImGui::SeparatorText("Parameters");

	EnumAdapter<EndCondition>::Combo("EndCondition", &endCondition_);
	ImGuiHelper::ComboFromStrings("AnimationName", &animationName_, player_->GetAnimNames());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("distance", &distance_, 0.1f);

	// 終了後タイマー
	exitTimer_.ImGui("ExitTimer");

	// 座標
	lerpPos_.ImGui("LerpPos");
	// 回転
	lerpRotate_.ImGui("LerpRotate");
}

void PlayerMoveFrontAttack::FromJson(const Json& data) {

	endCondition_ = EnumAdapter<EndCondition>::FromString(data.value("EndCondition", "EndAnim")).value();
	animationName_ = data.value("AnimationName", "");
	nextAnimDuration_ = data.value("NextAnimDuration", 0.0f);
	distance_ = data.value("Distance", 0.0f);

	exitTimer_.FromJson(data.value("ExitTimer", Json()));
	lerpPos_.timer.FromJson(data["LerpPosTimer"]);
	lerpRotate_.timer.FromJson(data["LerpRotateTimer"]);
}

void PlayerMoveFrontAttack::ToJson(Json& data) {

	data["EndCondition"] = EnumAdapter<EndCondition>::ToString(endCondition_);
	data["AnimationName"] = animationName_;
	data["NextAnimDuration"] = nextAnimDuration_;
	data["Distance"] = distance_;

	exitTimer_.ToJson(data["ExitTimer"]);
	lerpPos_.timer.ToJson(data["LerpPosTimer"]);
	lerpRotate_.timer.ToJson(data["LerpRotateTimer"]);
}
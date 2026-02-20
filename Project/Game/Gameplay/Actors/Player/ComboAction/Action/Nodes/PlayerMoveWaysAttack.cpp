#include "PlayerMoveWaysAttack.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerMoveWaysAttack classMethods
//============================================================================

void PlayerMoveWaysAttack::SetProgress([[maybe_unused]] float progress) {
}

bool PlayerMoveWaysAttack::IsFinished() const {

	return exitTimer_.IsReached();
}

float PlayerMoveWaysAttack::GetTotalTime() const {

	// アニメーション時間+終了後時間
	return player_->GetAnimationDuration("player_attack_2nd") + exitTimer_.target_;
}

void PlayerMoveWaysAttack::Enter() {

	// タイマーリセット
	lerpRotate_.timer.Reset();
	exitTimer_.Reset();

	// アニメーションの再生
	player_->SetNextAnimation("player_attack_2nd", false, nextAnimDuration_);

	// 初期化
	currentIndex_ = 0;
	segmentTimer_ = 0.0f;
	// 各区間の時間を分割する
	segmentTime_ = attackPosLerpTime_ / static_cast<float>(kNumSegments);

	//============================================================================
	// 移動開始位置、目標位置の設定
	//============================================================================
	{
		// 距離に応じて各補間点を計算する
		if (areaChecker_->IsInRange(AreaReactionType::LerpPos)) {

			// 範囲内のとき
			CalcWayPoints(wayPoints_);
		} else {

			// 範囲外のとき
			CalcApproachWayPoints(wayPoints_);
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

void PlayerMoveWaysAttack::Update() {

	// アニメーション終了後、時間経過を進めて終了させる
	if (player_->IsAnimationFinished()) {

		exitTimer_.Update();
	}

	// 区間補間が終了したかチェック
	bool finished = LerpAlongSegments();
	if (!finished) {
		return;
	}

	// 区間完了ごとに次の経路を決める
	if (approachPhase_) {

		if (!loopApproach_) {
			if (targetTranslation_.has_value()) {

				player_->SetTranslation(*targetTranslation_);
			}
			// これ以上補間処理を行わない
			approachPhase_ = false;
			return;
		}

		// 範囲内になったら敵補間へ切り替え
		if (areaChecker_->IsInRange(AreaReactionType::LerpPos)) {

			// 範囲内
			approachPhase_ = false;
			CalcWayPoints(wayPoints_);
			startTranslation_ = player_->GetTranslation();
		} else {

			// 範囲外
			CalcApproachWayPoints(wayPoints_);
			startTranslation_ = player_->GetTranslation();
		}

		// リセット
		currentIndex_ = 0;
		segmentTimer_ = 0.0f;
		segmentTime_ = attackPosLerpTime_ / static_cast<float>(kNumSegments);
	} else {

		// 範囲内の経路完走後は最終点へ固定
		player_->SetTranslation(*targetTranslation_);
	}
}

void PlayerMoveWaysAttack::Exit() {

	// リセット
	exitTimer_.Reset();
}

void PlayerMoveWaysAttack::CalcWayPoints(std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints) {

	// 目標座標を設定
	startTranslation_ = player_->GetTranslation();
	SakuEngine::Vector3 enemyPos = SakuEngine::Math::GetFlattenPos3D(*attackTarget_);
	// プレイヤーのY座標と合わせる
	enemyPos.y = startTranslation_.y;
	// 目標座標を敵の方向に設定
	targetTranslation_ = enemyPos - SakuEngine::Math::GetDirection3D(*player_, *attackTarget_) * offsetDistance_;

	//距離に応じて振れ幅を変更する
	float distance = (*targetTranslation_ - startTranslation_).Length();
	float swayLength = (std::max)(0.0f, (areaChecker_->GetRange(AreaReactionType::LerpPos) - distance)) * swayRate_;

	// 補間先を設定する
	CalcWayPointsToTarget(startTranslation_, *targetTranslation_, leftPointAngle_, rightPointAngle_, swayLength, dstWayPoints);
}

void PlayerMoveWaysAttack::CalcWayPointsToTarget(const SakuEngine::Vector3& start,
	const SakuEngine::Vector3& target, float leftT, float rightT, float swayLength,
	std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints) {

	SakuEngine::Vector3 direction = SakuEngine::Vector3(target - start).Normalize();
	SakuEngine::Vector3 right = SakuEngine::Vector3::Cross(direction, Direction::Get(Direction3D::Up)).Normalize();

	// 左右の補間点を計算
	dstWayPoints[0] = SakuEngine::Vector3::Lerp(start, target, leftT) - right * swayLength;  // 左
	dstWayPoints[1] = SakuEngine::Vector3::Lerp(start, target, rightT) + right * swayLength; // 右
	dstWayPoints[2] = target;
}

void PlayerMoveWaysAttack::CalcApproachWayPoints(
	std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints) {

	// プレイヤーの前方向に向かってジグザグ移動させる
	startTranslation_ = player_->GetTranslation();
	SakuEngine::Vector3 forward = player_->GetTransform().GetForward().Normalize();
	forward.y = 0.0f;
	forward = forward.Normalize();
	SakuEngine::Vector3 target = startTranslation_ + forward * approachForwardDistance_;
	targetTranslation_ = target;

	// 補間先を設定する
	CalcWayPointsToTarget(startTranslation_, target,
		approachLeftPointAngle_, approachRightPointAngle_,
		approachSwayLength_, dstWayPoints);
}

bool PlayerMoveWaysAttack::LerpAlongSegments() {

	if (wayPoints_.size() <= currentIndex_) {
		if (targetTranslation_.has_value()) {

			player_->SetTranslation(*targetTranslation_);
		}
		return true;
	}

	// 区間更新
	segmentTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
	float t = std::clamp(segmentTimer_ / segmentTime_, 0.0f, 1.0f);
	t = EasedValue(attackPosEaseType_, t);

	SakuEngine::Vector3 segStart = (currentIndex_ == 0) ? startTranslation_ : wayPoints_[currentIndex_ - 1];
	SakuEngine::Vector3 segEnd = wayPoints_[currentIndex_];

	SakuEngine::Vector3 pos = SakuEngine::Vector3::Lerp(segStart, segEnd, t);
	player_->SetTranslation(pos);

	// 補間が終了したら次の区間に進める
	if (segmentTime_ <= segmentTimer_) {

		segmentTimer_ = 0.0f;
		++currentIndex_;
		return (currentIndex_ >= wayPoints_.size());
	}
	return false;
}

void PlayerMoveWaysAttack::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("attackPosLerpTime", &attackPosLerpTime_, 0.01f);
	ImGui::DragFloat("swayRate", &swayRate_, 0.01f);
	ImGui::DragFloat("leftPointAngle", &leftPointAngle_, 0.01f);
	ImGui::DragFloat("rightPointAngle", &rightPointAngle_, 0.01f);
	ImGui::DragFloat("offsetDistance", &offsetDistance_, 0.1f);

	// 範囲内
	CalcWayPoints(debugWayPoints_);
	{
		SakuEngine::LineRenderer* renderer = SakuEngine::LineRenderer::GetInstance();
		SakuEngine::Vector3 prev = player_->GetTranslation();
		for (auto& p : debugWayPoints_) {
			p.y = 2.0f;
			renderer->Get3D()->DrawSphere(8, 2.0f, p, SakuEngine::Color::Red());
			renderer->Get3D()->DrawLine(p, p + SakuEngine::Vector3(0, 2, 0), SakuEngine::Color::White());
			renderer->Get3D()->DrawLine(prev, p, SakuEngine::Color::White());
			prev = p;
		}
	}

	// 範囲外
	ImGui::SeparatorText("Approach (Out of Range)");
	ImGui::DragFloat("approachForwardDistance", &approachForwardDistance_, 0.1f);
	ImGui::DragFloat("approachSwayLength", &approachSwayLength_, 0.01f);
	ImGui::DragFloat("approachLeftPointAngle", &approachLeftPointAngle_, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("approachRightPointAngle", &approachRightPointAngle_, 0.01f, 0.0f, 1.0f);

	CalcApproachWayPoints(debugApproachWayPoints_);
	{
		SakuEngine::LineRenderer* renderer = SakuEngine::LineRenderer::GetInstance();
		SakuEngine::Vector3 prev = player_->GetTranslation();
		for (auto& p : debugApproachWayPoints_) {

			p.y = 2.0f;
			renderer->Get3D()->DrawSphere(8, 2.0f, p, SakuEngine::Color::Green());
			renderer->Get3D()->DrawLine(p, p + SakuEngine::Vector3(0.0f, 2.0f, 0.0f), SakuEngine::Color::White());
			renderer->Get3D()->DrawLine(prev, p, SakuEngine::Color::White());
			prev = p;
		}
	}

	Easing::SelectEasingType(attackPosEaseType_);
	exitTimer_.ImGui("ExitTimer");
	lerpRotate_.ImGui("LerpRotate");
}

void PlayerMoveWaysAttack::FromJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.0f);
	swayRate_ = data.value("swayRate_", 0.0f);
	attackPosLerpTime_ = data.value("attackPosLerpTime_", 0.0f);
	leftPointAngle_ = data.value("leftPointAngle_", 0.0f);
	rightPointAngle_ = data.value("rightPointAngle_", 0.0f);
	approachForwardDistance_ = data.value("approachForwardDistance_", 0.0f);
	approachSwayLength_ = data.value("approachSwayLength_", 0.0f);
	approachLeftPointAngle_ = data.value("approachLeftPointAngle_", 0.0f);
	approachRightPointAngle_ = data.value("approachRightPointAngle_", 0.0f);
	offsetDistance_ = data.value("offsetDistance_", 0.0f);
	attackPosEaseType_ = EnumAdapter<EasingType>::FromString(data.value("AttackPosEaseType", "Linear")).value();

	exitTimer_.FromJson(data.value("ExitTimer", Json()));
	lerpRotate_.timer.FromJson(data["LerpRotateTimer"]);
}

void PlayerMoveWaysAttack::ToJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["swayRate_"] = swayRate_;
	data["attackPosLerpTime_"] = attackPosLerpTime_;
	data["leftPointAngle_"] = leftPointAngle_;
	data["rightPointAngle_"] = rightPointAngle_;
	data["approachForwardDistance_"] = approachForwardDistance_;
	data["approachSwayLength_"] = approachSwayLength_;
	data["approachLeftPointAngle_"] = approachLeftPointAngle_;
	data["approachRightPointAngle_"] = approachRightPointAngle_;
	data["offsetDistance_"] = offsetDistance_;
	data["AttackPosEaseType"] = EnumAdapter<EasingType>::ToString(attackPosEaseType_);

	exitTimer_.ToJson(data["ExitTimer"]);
	lerpRotate_.timer.ToJson(data["LerpRotateTimer"]);
}
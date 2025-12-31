#include "PlayerAttack_2ndState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerAttack_2ndState classMethods
//============================================================================

void PlayerAttack_2ndState::CreateEffect() {

	// 剣エフェクト作成
	// 1段目
	slash1stEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	slash1stEffect_->Init("slashEffect1st", "PlayerEffect");
	slash1stEffect_->LoadJson("GameEffectGroup/Player/playerAttackSlashEffect_0.json");
	// 2段目
	slash2ndEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	slash2ndEffect_->Init("slashEffect2nd", "PlayerEffect");
	slash2ndEffect_->LoadJson("GameEffectGroup/Player/playerAttackSlashEffect_1.json");

	// 親の設定
	slash1stEffect_->SetParent("playerAttackSlash_0", player_->GetTransform());
	slash2ndEffect_->SetParent("playerAttackSlash_1", player_->GetTransform());
}

void PlayerAttack_2ndState::Enter() {

	player_->SetNextAnimation("player_attack_2nd", false, nextAnimDuration_);
	canExit_ = false;

	// 距離を比較
	const SakuEngine::Vector3 playerPos = player_->GetTranslation();

	// 初期化
	currentIndex_ = 0;
	segmentTimer_ = 0.0f;
	segmentTime_ = attackPosLerpTime_ / 3.0f;

	if (attackPosLerpCircleRange_ < SakuEngine::Math::GetDistance3D(*player_, *bossEnemy_, true, true)) {

		// 範囲外のとき
		CalcApproachWayPoints(wayPoints_);
	} else {

		// 範囲内のとき
		CalcWayPoints(wayPoints_);
	}
	startTranslation_ = playerPos;

	// 回転補間させるかの設定
	approachPhase_ = CheckInRange(attackLookAtCircleRange_, SakuEngine::Math::GetDistance3D(*player_, *bossEnemy_, true, true));

	// 剣エフェクトの発生
	slash1stEffect_->Emit(player_->GetRotation() * slash1stEffectOffset_);
}

void PlayerAttack_2ndState::Update() {

	// animationが終わったかチェック
	canExit_ = player_->IsAnimationFinished();
	// animationが終わったら時間経過を進める
	if (canExit_) {

		exitTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
	}

	// 区間補間処理
	const bool finished = LerpAlongSegments();
	if (!finished) {
		return;
	}

	// 区間完了ごとに次の経路を決める
	if (approachPhase_) {

		if (!loopApproach_) {
			if (targetTranslation_.has_value()) {

				player_->SetTranslation(*targetTranslation_);
			}
			approachPhase_ = false; // これ以上補間処理を行わない
			return;
		}

		// 範囲内になったら敵補間へ切り替え
		if (SakuEngine::Math::GetDistance3D(*player_, *bossEnemy_, true, true) <= attackPosLerpCircleRange_) {

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
		segmentTime_ = attackPosLerpTime_ / 3.0f;
	} else {

		// 範囲内の経路完走後は最終点へ固定
		player_->SetTranslation(*targetTranslation_);
	}
}

void PlayerAttack_2ndState::UpdateAlways() {

	// 剣エフェクトの更新、親の回転を設定する
	// 1段目
	slash1stEffect_->SetParentRotation("playerAttackSlash_0",
		SakuEngine::Quaternion::Normalize(player_->GetRotation()), ParticleUpdateModuleID::Rotation);
	slash1stEffect_->Update();
	// 2段目
	slash2ndEffect_->SetParentRotation("playerAttackSlash_1",
		SakuEngine::Quaternion::Normalize(player_->GetRotation()), ParticleUpdateModuleID::Rotation);
	slash2ndEffect_->Update();
}

void PlayerAttack_2ndState::CalcWayPoints(std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints) {

	// 目標座標を設定
	startTranslation_ = player_->GetTranslation();
	SakuEngine::Vector3 enemyPos = SakuEngine::Math::GetFlattenPos3D(*bossEnemy_);
	// プレイヤーのY座標と合わせる
	enemyPos.y = startTranslation_.y;
	// 目標座標を敵の方向に設定
	targetTranslation_ = enemyPos - SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_) * attackOffsetTranslation_;

	//距離に応じて振れ幅を変更する
	float distance = (*targetTranslation_ - startTranslation_).Length();
	float swayLength = (std::max)(0.0f, (attackPosLerpCircleRange_ - distance)) * swayRate_;

	// 補間先を設定する
	CalcWayPointsToTarget(startTranslation_, *targetTranslation_, leftPointAngle_, rightPointAngle_, swayLength, dstWayPoints);
}

void PlayerAttack_2ndState::CalcWayPointsToTarget(const SakuEngine::Vector3& start, const SakuEngine::Vector3& target,
	float leftT, float rightT, float swayLength, std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints) {

	SakuEngine::Vector3 direction = SakuEngine::Vector3(target - start).Normalize();
	SakuEngine::Vector3 right = SakuEngine::Vector3::Cross(direction, Direction::Get(Direction3D::Up)).Normalize();

	// 左右の補間点を計算
	dstWayPoints[0] = SakuEngine::Vector3::Lerp(start, target, leftT) - right * swayLength;  // 左
	dstWayPoints[1] = SakuEngine::Vector3::Lerp(start, target, rightT) + right * swayLength; // 右
	dstWayPoints[2] = target;
}

void PlayerAttack_2ndState::CalcApproachWayPoints(std::array<SakuEngine::Vector3, kNumSegments>& dstWayPoints) {

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

bool PlayerAttack_2ndState::LerpAlongSegments() {

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

		// 剣エフェクトの発生
		if (currentIndex_ == 0) {

			slash2ndEffect_->Emit(player_->GetRotation() * slash2ndEffectOffset_);
		}

		segmentTimer_ = 0.0f;
		++currentIndex_;
		return (currentIndex_ >= wayPoints_.size());
	}
	return false;
}

void PlayerAttack_2ndState::Exit() {

	// リセット
	attackPosLerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
}

void PlayerAttack_2ndState::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("targetCameraRotateX", &targetCameraRotateX_, 0.01f);

	ImGui::DragFloat("swayRate", &swayRate_, 0.01f);
	ImGui::DragFloat("leftPointAngle", &leftPointAngle_, 0.01f);
	ImGui::DragFloat("rightPointAngle", &rightPointAngle_, 0.01f);
	ImGui::DragFloat3("slash1stEffectOffset", &slash1stEffectOffset_.x, 0.1f);
	ImGui::DragFloat3("slash2ndEffectOffset", &slash2ndEffectOffset_.x, 0.1f);

	PlayerBaseAttackState::ImGui();

	// 範囲内
	CalcWayPoints(debugWayPoints_);
	{
		SakuEngine::LineRenderer* renderer = SakuEngine::LineRenderer::GetInstance();
		SakuEngine::Vector3 prev = player_->GetTranslation();
		for (auto& p : debugWayPoints_) {
			p.y = 2.0f;
			renderer->DrawSphere(8, 2.0f, p, SakuEngine::Color::Red());
			renderer->DrawLine3D(p, p + SakuEngine::Vector3(0, 2, 0), SakuEngine::Color::White());
			renderer->DrawLine3D(prev, p, SakuEngine::Color::White());
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
			renderer->DrawSphere(8, 2.0f, p, SakuEngine::Color::Green());
			renderer->DrawLine3D(p, p + SakuEngine::Vector3(0, 2, 0), SakuEngine::Color::White());
			renderer->DrawLine3D(prev, p, SakuEngine::Color::White());
			prev = p;
		}
	}
}

void PlayerAttack_2ndState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "exitTime_");
	swayRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "swayRate_");
	leftPointAngle_ = SakuEngine::JsonAdapter::GetValue<float>(data, "leftPointAngle_");
	rightPointAngle_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rightPointAngle_");

	slash1stEffectOffset_ = SakuEngine::Vector3::FromJson(data.value("slash1stEffectOffset_", Json()));
	slash2ndEffectOffset_ = SakuEngine::Vector3::FromJson(data.value("slash2ndEffectOffset_", Json()));

	targetCameraRotateX_ = data.value("targetCameraRotateX_", 0.0f);

	approachForwardDistance_ = data.value("approachForwardDistance_", approachForwardDistance_);
	approachSwayLength_ = data.value("approachSwayLength_", approachSwayLength_);
	approachLeftPointAngle_ = data.value("approachLeftPointAngle_", approachLeftPointAngle_);
	approachRightPointAngle_ = data.value("approachRightPointAngle_", approachRightPointAngle_);

	PlayerBaseAttackState::ApplyJson(data);
}

void PlayerAttack_2ndState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;
	data["swayRate_"] = swayRate_;
	data["leftPointAngle_"] = leftPointAngle_;
	data["rightPointAngle_"] = rightPointAngle_;
	data["targetCameraRotateX_"] = targetCameraRotateX_;

	data["slash1stEffectOffset_"] = slash1stEffectOffset_.ToJson();
	data["slash2ndEffectOffset_"] = slash2ndEffectOffset_.ToJson();

	data["approachForwardDistance_"] = approachForwardDistance_;
	data["approachSwayLength_"] = approachSwayLength_;
	data["approachLeftPointAngle_"] = approachLeftPointAngle_;
	data["approachRightPointAngle_"] = approachRightPointAngle_;

	PlayerBaseAttackState::SaveJson(data);
}

bool PlayerAttack_2ndState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}
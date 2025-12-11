#include "PlayerBaseAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	PlayerBaseAttackState classMethods
//============================================================================

void PlayerBaseAttackState::UpdateTimer(SakuEngine::StateTimer& timer) {

	// 外部による更新がないときのみ
	if (!externalActive_) {

		timer.Update();
	}
}

void PlayerBaseAttackState::AttackAssist(Player& player, bool onceTarget) {

	// 時間経過
	attackPosLerpTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
	float lerpT = std::clamp(attackPosLerpTimer_ / attackPosLerpTime_, 0.0f, 1.0f);
	lerpT = EasedValue(attackPosEaseType_, lerpT);

	// 座標、距離を取得
	SakuEngine::Vector3 playerPos = player.GetTranslation();
	SakuEngine::Vector3 enemyPos = PlayerIState::GetBossEnemyFixedYPos();
	// y座標を合わせる
	enemyPos.y = playerPos.y;
	// プレイヤーとボス敵の距離、向きを取得
	float distance = PlayerIState::GetDistanceToBossEnemy();
	SakuEngine::Vector3 direction = PlayerIState::GetDirectionToBossEnemy();

	// 補間先を設定
	if (onceTarget) {
		if (!targetTranslation_.has_value() &&
			!targetRotation_.has_value()) {

			targetTranslation_ = enemyPos - direction * attackOffsetTranslation_;
			targetRotation_ = SakuEngine::Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up));
		}
	} else {

		targetTranslation_ = enemyPos - direction * attackOffsetTranslation_;
		targetRotation_ = SakuEngine::Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up));
	}

	// 指定円の中に敵がいれば敵の座標まで補間する
	if (CheckInRange(attackPosLerpCircleRange_, distance)) {

		// 補間先
		SakuEngine::Vector3 translation = SakuEngine::Vector3::Lerp(playerPos, *targetTranslation_, std::clamp(lerpT, 0.0f, 1.0f));
		player.SetTranslation(translation);
	}

	// 指定円の中に敵がいれば敵の方向に向かせる
	if (CheckInRange(attackLookAtCircleRange_, distance)) {

		SakuEngine::Quaternion currentRotation = player.GetRotation();
		player.SetRotation(SakuEngine::Quaternion::Slerp(currentRotation, *targetRotation_, std::clamp(rotationLerpRate_, 0.0f, 1.0f)));
	}
}

bool PlayerBaseAttackState::CheckInRange(float range, float distance) {

	bool result = range > epsilon_ && distance <= range;

	// ボスが無効状態なら常にfalseを返す
	if (bossEnemy_->IsInvincible()) {
		result = false;
	}

	return result;
}

SakuEngine::Vector3 PlayerBaseAttackState::GetPlayerOffsetPos(
	const Player& player, const SakuEngine::Vector3& offsetTranslation) const {

	SakuEngine::Vector3 offset = SakuEngine::Vector3::Transform(offsetTranslation,
		SakuEngine::Quaternion::MakeRotateMatrix(player.GetRotation()));

	return player.GetTranslation() + offset;
}

SakuEngine::Matrix4x4 PlayerBaseAttackState::GetPlayerOffsetRotation(
	const Player& player, const SakuEngine::Vector3& offsetRotation) const {

	// playerの回転
	SakuEngine::Matrix4x4 playerRotation = SakuEngine::Quaternion::MakeRotateMatrix(player.GetRotation());
	// オフセット回転
	SakuEngine::Matrix4x4 offsetMatrix = SakuEngine::Matrix4x4::MakeRotateMatrix(offsetRotation);

	return playerRotation * offsetMatrix;
}

void PlayerBaseAttackState::SetTimerByOverall(SakuEngine::StateTimer& timer, float overall,
	float start, float end, EasingType easing) {

	timer.t_ = SakuEngine::Algorithm::MapOverallToLocal(overall, start, end);
	timer.easedT_ = EasedValue(easing, timer.t_);
}

void PlayerBaseAttackState::DrawAttackOffset(const Player& player) {

	SakuEngine::LineRenderer* lineRenderer = SakuEngine::LineRenderer::GetInstance();

	// 座標、距離を取得
	SakuEngine::Vector3 playerPos = player.GetTranslation();
	playerPos.y = 2.0f;
	SakuEngine::Vector3 enemyPos = bossEnemy_->GetTranslation();
	enemyPos.y = 2.0f;
	SakuEngine::Vector3 direction = (enemyPos - playerPos).Normalize();
	SakuEngine::Vector3 target = enemyPos - direction * attackOffsetTranslation_;

	lineRenderer->DrawLine3D(playerPos, target, SakuEngine::Color::Red());
	lineRenderer->DrawSphere(8, 2.0f, target, SakuEngine::Color::Red());
}

void PlayerBaseAttackState::ResetTarget() {

	// 補間目標をリセット
	attackPosLerpTimer_ = 0.0f;
	targetTranslation_ = std::nullopt;
	targetRotation_ = std::nullopt;
}

void PlayerBaseAttackState::ImGui(const Player& player) {

	ImGui::DragFloat("attackPosLerpCircleRange", &attackPosLerpCircleRange_, 0.1f);
	ImGui::DragFloat("attackLookAtCircleRange", &attackLookAtCircleRange_, 0.1f);
	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("attackPosLerpTime", &attackPosLerpTime_, 0.01f);
	Easing::SelectEasingType(attackPosEaseType_);

	DrawAttackOffset(player);

	SakuEngine::LineRenderer* lineRenderer = SakuEngine::LineRenderer::GetInstance();

	SakuEngine::Vector3 center = player.GetTranslation();
	center.y = 2.0f;
	lineRenderer->DrawCircle(8, attackPosLerpCircleRange_, center, SakuEngine::Color::Red());
	lineRenderer->DrawCircle(8, attackLookAtCircleRange_, center, SakuEngine::Color::Blue());
}

void PlayerBaseAttackState::ApplyJson(const Json& data) {

	attackPosLerpCircleRange_ = SakuEngine::JsonAdapter::GetValue<float>(data, "attackPosLerpCircleRange_");
	attackLookAtCircleRange_ = SakuEngine::JsonAdapter::GetValue<float>(data, "attackLookAtCircleRange_");
	attackOffsetTranslation_ = SakuEngine::JsonAdapter::GetValue<float>(data, "attackOffsetTranslation_");
	attackPosLerpTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "attackPosLerpTime_");
	attackPosEaseType_ = static_cast<EasingType>(SakuEngine::JsonAdapter::GetValue<int>(data, "attackPosEaseType_"));
}

void PlayerBaseAttackState::SaveJson(Json& data) {

	data["attackPosLerpCircleRange_"] = attackPosLerpCircleRange_;
	data["attackLookAtCircleRange_"] = attackLookAtCircleRange_;
	data["attackOffsetTranslation_"] = attackOffsetTranslation_;
	data["attackPosLerpTime_"] = attackPosLerpTime_;
	data["attackPosEaseType_"] = static_cast<int>(attackPosEaseType_);
}
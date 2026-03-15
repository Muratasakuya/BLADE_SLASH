#include "PlayerBaseAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	PlayerBaseAttackState classMethods
//============================================================================

void PlayerBaseAttackState::AttackAssist(bool onceTarget, bool isOnlyAssistRotation) {

	// 時間経過
	attackPosLerpTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
	float lerpT = std::clamp(attackPosLerpTimer_ / attackPosLerpTime_, 0.0f, 1.0f);
	lerpT = EasedValue(attackPosEaseType_, lerpT);

	// 座標、距離を取得
	SakuEngine::Vector3 playerPos = SakuEngine::Math::GetFlattenPos3D(*player_);
	SakuEngine::Vector3 enemyPos = SakuEngine::Math::GetFlattenPos3D(*bossEnemy_);
	// プレイヤーとボス敵の距離、向きを取得
	float distance = SakuEngine::Math::GetDistance3D(*player_, *bossEnemy_, true, true);
	SakuEngine::Vector3 direction = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);

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
	if (!isOnlyAssistRotation && CheckInRange(attackPosLerpCircleRange_, distance)) {

		// 補間先
		SakuEngine::Vector3 translation = SakuEngine::Vector3::Lerp(playerPos, *targetTranslation_, std::clamp(lerpT, 0.0f, 1.0f));
		player_->SetTranslation(translation);
	}

	// 指定円の中に敵がいれば敵の方向に向かせる
	if (CheckInRange(attackLookAtCircleRange_, distance)) {

		SakuEngine::Quaternion currentRotation = player_->GetRotation();
		player_->SetRotation(SakuEngine::Quaternion::Slerp(currentRotation, *targetRotation_, std::clamp(rotationLerpRate_, 0.0f, 1.0f)));
	}
}

bool PlayerBaseAttackState::CheckInRange(float range, float distance) {

	bool result = range > Config::kEpsilon && distance <= range;
	return result;
}

void PlayerBaseAttackState::DrawAttackOffset() {

	SakuEngine::LineRenderer* lineRenderer = SakuEngine::LineRenderer::GetInstance();

	// 座標、距離を取得
	SakuEngine::Vector3 playerPos = player_->GetTranslation();
	playerPos.y = 2.0f;
	SakuEngine::Vector3 enemyPos = bossEnemy_->GetTranslation();
	enemyPos.y = 2.0f;
	SakuEngine::Vector3 direction = (enemyPos - playerPos).Normalize();
	SakuEngine::Vector3 target = enemyPos - direction * attackOffsetTranslation_;

	lineRenderer->Get3D()->DrawLine(playerPos, target, SakuEngine::Color::Red());
	lineRenderer->Get3D()->DrawSphere(8, 2.0f, target, SakuEngine::Color::Red());
}

void PlayerBaseAttackState::ImGui() {

	ImGui::DragFloat("attackPosLerpCircleRange", &attackPosLerpCircleRange_, 0.1f);
	ImGui::DragFloat("attackLookAtCircleRange", &attackLookAtCircleRange_, 0.1f);
	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("attackPosLerpTime", &attackPosLerpTime_, 0.01f);
	Easing::SelectEasingType(attackPosEaseType_);

	DrawAttackOffset();

	SakuEngine::LineRenderer* lineRenderer = SakuEngine::LineRenderer::GetInstance();

	SakuEngine::Vector3 center = player_->GetTranslation();
	center.y = 2.0f;
	lineRenderer->Get3D()->DrawCircle(8, attackPosLerpCircleRange_, center, SakuEngine::Color::Red());
	lineRenderer->Get3D()->DrawCircle(8, attackLookAtCircleRange_, center, SakuEngine::Color::Blue());
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
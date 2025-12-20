#include "PlayerAvoidSatate.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerAvoidSatate classMethods
//============================================================================

void PlayerAvoidSatate::Enter() {

	player_->SetNextAnimation("player_avoid", false, nextAnimDuration_);

	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	const SakuEngine::Vector3 enemyPos = bossEnemy_->GetTranslation();
	// 向き
	SakuEngine::Vector3 direction = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);

	// 補間座標を設定する
	startPos_ = playerPos;
	targetPos_ = playerPos + direction * moveDistance_;

	// 敵の方向を向かせる
	player_->SetRotation(SakuEngine::Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up)));

	// 回避開始
	isAvoiding_ = true;

	canExit_ = false;
}

void PlayerAvoidSatate::Update() {

	// 時間を進める
	lerpTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
	float lerpT = lerpTimer_ / lerpTime_;
	lerpT = EasedValue(easingType_, lerpT);

	// 座標を補間
	SakuEngine::Vector3 translation = SakuEngine::Vector3::Lerp(startPos_, targetPos_, lerpT);

	// 座標を設定
	player_->SetTranslation(translation);

	// 時間が経過しきったら遷移可能状態にする
	if (lerpTime_ < lerpTimer_) {

		canExit_ = true;

		// 回避終了
		isAvoiding_ = false;
	}
}

void PlayerAvoidSatate::Exit() {

	// リセット
	lerpTimer_ = 0.0f;
	canExit_ = false;
	// 回避終了
	isAvoiding_ = false;
}

void PlayerAvoidSatate::ImGui() {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text(std::format("isAvoiding: {}", isAvoiding_).c_str());

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
	ImGui::DragFloat("moveDistance", &moveDistance_, 0.1f);
	Easing::SelectEasingType(easingType_);

	const SakuEngine::Vector3 playerPos = player_->GetTranslation();
	const SakuEngine::Vector3 enemyPos = bossEnemy_->GetTranslation();
	// 向き
	SakuEngine::Vector3 direction = (enemyPos - playerPos).Normalize();

	// 補間座標を設定する
	SakuEngine::Vector3 startPos = playerPos;
	SakuEngine::Vector3 targetPos = playerPos + direction * moveDistance_;
	startPos.y = 4.0f;
	targetPos.y = 4.0f;

	SakuEngine::LineRenderer::GetInstance()->DrawLine3D(
		startPos, targetPos, SakuEngine::Color::Red());
}

void PlayerAvoidSatate::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	lerpTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpTime_");
	moveDistance_ = SakuEngine::JsonAdapter::GetValue<float>(data, "moveDistance_");
	easingType_ = static_cast<EasingType>(SakuEngine::JsonAdapter::GetValue<int>(data, "easingType_"));
}

void PlayerAvoidSatate::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["lerpTime_"] = lerpTime_;
	data["moveDistance_"] = moveDistance_;
	data["easingType_"] = static_cast<int>(easingType_);
}
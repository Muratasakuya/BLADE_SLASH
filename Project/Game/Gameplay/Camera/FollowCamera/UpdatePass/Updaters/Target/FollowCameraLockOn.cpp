#include "FollowCameraLockOn.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Config.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	FollowCameraLockOn classMethods
//============================================================================

void FollowCameraLockOn::Init() {

	// エリアチェッカー初期化
	areaChecker_ = std::make_unique<ObjectAreaChecker>();
	areaChecker_->Init("Camera/Follow/lockOnAreaChecker.json");

	// json適用
	ApplyJson();
}

void FollowCameraLockOn::BindDependencies(const FollowCameraDependencies& dependencies) {

	// 依存設定
	dependencies_ = dependencies;
	// エリアチェッカーにオブジェクト設定
	areaChecker_->SetAnchor(dependencies_.player);
	areaChecker_->SetTarget(dependencies_.bossEnemy);
}

void FollowCameraLockOn::Execute(FollowCameraContext& context,
	const FollowCameraFrameService& service, float deltaTime) {

	// エリアチェッカー更新
	areaChecker_->Update();

	// 条件を満たしていない場合敵の方にロックオンしない
	if (!IsLockOnAvailable(service)) {
		return;
	}

	// 現在の回転から目標回転への補間
	Quaternion currentRotation = Quaternion::Normalize(context.cameraRotation);
	context.cameraRotation = Quaternion::Slerp(currentRotation, GetTargetRotation(), lookLerpRate_ * deltaTime);

	context.cameraRotation = Quaternion::Normalize(context.cameraRotation);
}

void FollowCameraLockOn::Begin([[maybe_unused]] FollowCameraContext& context) {

	// 最短方向を取得
	// AreaCheckMethodに応じて基準点から見た方向を決定
	float yawDelta = Math::YawSignedDelta(Quaternion::Normalize(context.cameraRotation),
		dependencies_.bossEnemy->GetRotation());
	if (std::abs(yawDelta) <= Config::kEpsilon) {

		// どちらでも良いので右にする
		lookYawDirection_ = AnchorToDirection2D::Right;
	} else {

		// 最短方向
		lookYawDirection_ = (0.0f < yawDelta) ? AnchorToDirection2D::Right : AnchorToDirection2D::Left;
	}
}

void FollowCameraLockOn::ImGui() {

	if (ImGui::Button("Save Json##FollowCameraLockOn")) {

		SaveJson();
	}

	ImGui::SeparatorText("Config");

	ImGui::Text("direction: %s", EnumAdapter<AnchorToDirection2D>::ToString(lookYawDirection_));
	Quaternion targetRotation = GetTargetRotation();
	ImGui::Text("targetRotation: %.2f, %.2f, %.2f, %.2f", targetRotation.x, targetRotation.y, targetRotation.z, targetRotation.w);

	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("targetXRotation", &targetXRotation_, 0.01f);
	ImGui::DragFloat("lookYawOffset", &lookYawOffset_, 0.001f);
	ImGui::DragFloat("lookLerpRate", &lookLerpRate_, 0.01f, 0.0f, 10.0f);

	if (ImGui::CollapsingHeader("AreaChecker")) {

		areaChecker_->ImGui();
	}
}

void FollowCameraLockOn::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Follow/lockOn.json", data)) {
		return;
	}

	lookYawDirection_ = EnumAdapter<AnchorToDirection2D>::FromString(data.value("lookYawDirection_", "Left")).value();
	targetXRotation_ = data.value("targetXRotation_", targetXRotation_);
	lookYawOffset_ = data.value("lookYawOffset_", lookYawOffset_);
	lookLerpRate_ = data.value("lookLerpRate_", lookLerpRate_);
}

void FollowCameraLockOn::SaveJson() {

	Json data;

	data["lookYawDirection_"] = EnumAdapter<AnchorToDirection2D>::ToString(lookYawDirection_);
	data["targetXRotation_"] = targetXRotation_;
	data["lookYawOffset_"] = lookYawOffset_;
	data["lookLerpRate_"] = lookLerpRate_;

	JsonAdapter::Save("Camera/Follow/lockOn.json", data);
}

SakuEngine::Quaternion FollowCameraLockOn::GetTargetRotation() const {

	// プレイヤーから敵への向き
	Vector3 direction = Math::GetDirection3D(*dependencies_.player, *dependencies_.bossEnemy);

	// Y軸の回転
	Quaternion yawRotation = Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up));
	// X軸回転
	Vector3 rightAxis = Vector3(yawRotation * Direction::Get(Direction3D::Right)).Normalize();
	Quaternion pitchRotation = Quaternion::Normalize(Quaternion::MakeAxisAngle(rightAxis, targetXRotation_));

	// 目標回転
	Quaternion targetRotation = Quaternion::Normalize(pitchRotation * yawRotation);

	// 最短のY軸補間方向を取得
	float lookYawOffset = static_cast<float>(lookYawDirection_) * lookYawOffset_;
	// Y軸にオフセットをかける
	Quaternion yawOffsetRotation = Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Up), lookYawOffset);

	// Y軸オフセットをかけた目標回転
	targetRotation = Quaternion::Normalize(targetRotation * yawOffsetRotation);

	return targetRotation;
}

bool FollowCameraLockOn::IsLockOnAvailable(const FollowCameraFrameService& service) const {

	// プレイ中のみ
	if (service.sceneState != GameSceneState::PlayGame) {
		return false;
	}

	// プレイ中の制御
	// エリア範囲内
	// プレイヤーが攻撃していないとき
	bool available = areaChecker_->IsInRange(AreaReactionType::LerpCamera);
	available |= !dependencies_.player->IsAttacking();
	return available;
}
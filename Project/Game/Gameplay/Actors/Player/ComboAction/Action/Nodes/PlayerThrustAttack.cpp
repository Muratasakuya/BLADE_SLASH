#include "PlayerThrustAttack.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerThrustAttack classMethods
//============================================================================

void PlayerThrustAttack::SetProgress([[maybe_unused]] float progress) {
}

bool PlayerThrustAttack::IsFinished() const {

	return isFinished_;
}

float PlayerThrustAttack::GetTotalTime() const {

	return moveKeyframeObject_->GetTotalTime();
}

PlayerThrustAttack::PlayerThrustAttack() {

	// キーフレームオブジェクトの生成
	moveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	moveKeyframeObject_->Init("playerThrustMoveKey");

	// トランスフォームの生成
	BasePlayerMoveKeyAttack::Init("thrustAttack");
}

void PlayerThrustAttack::Enter() {

	// リセット
	isFinished_ = false;

	// アニメーション再生
	player_->SetNextAnimation("player_skilAttack_1st", false, nextAnimDuration_);

	// 敵が攻撃可能範囲にいるかチェックして目標を設定
	moveTransform_->parent = &player_->GetTransform();
	BasePlayerMoveKeyAttack::SetTargetByRange(*moveKeyframeObject_);

	// キーフレーム補間開始
	moveKeyframeObject_->StartLerp();
}

void PlayerThrustAttack::Update() {

	// キーフレーム更新
	moveKeyframeObject_->SelfUpdate();

	// 補間された回転、座標をプレイヤーに適用
	Vector3 currentTranslation = moveKeyframeObject_->GetCurrentTransform().translation;
	player_->SetTranslation(currentTranslation);
	// 回転は次の移動位置の方向を向くようにする
	// 方向
	Vector3 direction = Vector3(currentTranslation - preMovePos_).Normalize();
	Quaternion rotation = Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up));
	player_->SetRotation(Quaternion::Normalize(rotation));

	// 移動座標を更新する
	preMovePos_ = currentTranslation;

	// 補間処理が最後まで行ったら終了
	if (!moveKeyframeObject_->IsUpdating()) {

		isFinished_ = true;
	}
}

void PlayerThrustAttack::UpdateAlways() {

	// トランスフォーム更新
	BasePlayerMoveKeyAttack::UpdateTransform();

	// キーフレームオブジェクトの更新
	moveKeyframeObject_->UpdateKey();
}

void PlayerThrustAttack::Exit() {

	// リセット
	isFinished_ = false;
	// 補間を確実に終了させる
	moveKeyframeObject_->Reset();
}

void PlayerThrustAttack::ImGui() {

	ImGui::SeparatorText("Parameters");

	ImGui::Text(std::format("isFinished: {}", isFinished_).c_str());

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);

	ImGui::SeparatorText("Move Transform");

	if (ImGui::Button("Set Parent##key")) {

		moveTransform_->parent = &player_->GetTransform();
	}

	moveTransform_->ImGui(200.0f);

	LineRenderer::GetInstance()->Get3D()->DrawOBB(moveTransform_->GetWorldPos(),
		moveTransform_->scale, moveTransform_->rotation, Color::Cyan());

	ImGui::SeparatorText("KeyframeObject3D");

	if (ImGui::CollapsingHeader("MoveKeyframeObject")) {

		moveKeyframeObject_->ImGui();
	}
}

void PlayerThrustAttack::FromJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration", 0.2f);

	moveTransform_->FromJson(data.value("MoveTransform", Json()));
	moveKeyframeObject_->FromJson(data.value("MoveKey", Json()));
}

void PlayerThrustAttack::ToJson(Json& data) {

	data["nextAnimDuration"] = nextAnimDuration_;

	moveTransform_->ToJson(data["MoveTransform"]);
	moveKeyframeObject_->ToJson(data["MoveKey"]);
}
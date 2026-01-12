#include "PlayerJumpAttack.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerJumpAttack classMethods
//============================================================================

void PlayerJumpAttack::SetProgress([[maybe_unused]] float progress) {
}

bool PlayerJumpAttack::IsFinished() const {

	return exitTimer_.IsReached();
}

float PlayerJumpAttack::GetTotalTime() const {

	// ジャンプキーフレームの合計時間＋待機時間を返す
	return jumpKeyframeObject_->GetTotalTime()+ exitTimer_.target_;
}

PlayerJumpAttack::PlayerJumpAttack() {

	// キーフレームオブジェクトの生成
	jumpKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	jumpKeyframeObject_->Init("playerSkilJumpKey");

	// トランスフォームの生成
	BasePlayerMoveKeyAttack::Init("jumpAttack");
}

void PlayerJumpAttack::Enter() {

	// リセット
	exitTimer_.Reset();

	// アニメーション再生
	player_->SetNextAnimation("player_skilAttack_2nd", false, nextAnimDuration_);

	// 攻撃対象が範囲内に入っているかチェックする
	isInRange_ = areaChecker_->IsInRange(AreaReactionType::LerpPos);
	
	// 入っているかどうかでプレイヤーの回転を設定する
	Quaternion targetRotation = Quaternion::Identity();
	if (isInRange_) {

		// 範囲内なので敵の方向を向く回転を設定する
		Vector3 toEnemyDirection = Math::GetDirection3D(*player_, *attackTarget_);
		targetRotation = Quaternion::LookRotation(toEnemyDirection, Direction::Get(Direction3D::Up));
	} else {

		// 範囲外なので前方を向く回転を設定する
		// 移動後後ろ向いているのでGetBackで前方を取得
		Vector3 forward = player_->GetTransform().GetBack();
		targetRotation = Quaternion::LookRotation(forward, Direction::Get(Direction3D::Up));
	}
	// Enterした瞬間に回転を設定
	player_->SetRotation(Quaternion::Normalize(targetRotation));
	// 行列を更新
	player_->UpdateMatrix();
	moveTransform_->UpdateMatrix();

	// 敵が攻撃可能範囲にいるかチェックして目標を設定
	moveTransform_->parent = &player_->GetTransform();
	BasePlayerMoveKeyAttack::SetTargetByRange(*jumpKeyframeObject_);

	// ジャンプキーフレーム補間開始
	jumpKeyframeObject_->UpdateKey(true);
	jumpKeyframeObject_->StartLerp();
}

void PlayerJumpAttack::Update() {

	// トランスフォーム補間更新
	jumpKeyframeObject_->SelfUpdate();

	// 補間処理終了後状態を終了
	if (!jumpKeyframeObject_->IsUpdating()) {
		
		exitTimer_.Update(std::nullopt, false);
	} else {

		// 補間された座標をプレイヤーに適用
		SakuEngine::Vector3 currentTranslation = jumpKeyframeObject_->GetCurrentTransform().translation;
		player_->SetTranslation(currentTranslation);
	}
}

void PlayerJumpAttack::UpdateAlways() {

	// トランスフォーム更新
	BasePlayerMoveKeyAttack::UpdateTransform();

	// キーフレームオブジェクトの更新
	jumpKeyframeObject_->UpdateKey();
}

void PlayerJumpAttack::Exit() {

	// リセット
	exitTimer_.Reset();
	// 補間を確実に終了させる
	jumpKeyframeObject_->Reset();
}

void PlayerJumpAttack::ImGui() {

	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);

	exitTimer_.ImGui("ExitTimer");

	ImGui::SeparatorText("Move Transform");

	if (ImGui::Button("Set Parent##key")) {

		moveTransform_->parent = &player_->GetTransform();
	}

	moveTransform_->ImGui(200.0f);

	LineRenderer::GetInstance()->Get3D()->DrawOBB(moveTransform_->GetWorldPos(),
		moveTransform_->scale, moveTransform_->rotation, Color::Cyan());

	ImGui::SeparatorText("KeyframeObject3D");

	if (ImGui::CollapsingHeader("JumpKeyframeObject")) {

		jumpKeyframeObject_->ImGui();
	}
}

void PlayerJumpAttack::FromJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration", 0.2f);

	exitTimer_.FromJson(data.value("exitTimer_", Json()));
	moveTransform_->FromJson(data.value("MoveTransform", Json()));
	jumpKeyframeObject_->FromJson(data.value("JumpKeyframeObject", Json()));
}

void PlayerJumpAttack::ToJson(Json& data) {

	data["nextAnimDuration"] = nextAnimDuration_;

	exitTimer_.ToJson(data["exitTimer_"]);
	moveTransform_->ToJson(data["MoveTransform"]);
	jumpKeyframeObject_->ToJson(data["JumpKeyframeObject"]);
}
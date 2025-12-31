#include "PlayerCatchWeaponAttack.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerCatchWeaponAttack classMethods
//============================================================================

void PlayerCatchWeaponAttack::SetProgress([[maybe_unused]] float progress) {
}

bool PlayerCatchWeaponAttack::IsFinished() const {

	// 終了待機時間が経過したら終了
	return exitTimer_.IsReached();
}

float PlayerCatchWeaponAttack::GetTotalTime() const {

	// 座標補間時間+終了待機時間がそのまま最大時間
	return lerpPos_.timer.target_ + exitTimer_.target_;
}

void PlayerCatchWeaponAttack::Enter() {

	// リセット
	lerpPos_.timer.Reset();
	exitTimer_.Reset();

	// アニメーションの再生
	player_->SetNextAnimation("player_attack_3rd_Catch", false, nextAnimDuration_);

	// 補間開始座標
	lerpPos_.start = player_->GetTranslation();
	// 目標座標を剣の間の座標に設定する
	lerpPos_.target = (player_->GetWeapon(PlayerWeaponType::Left)->GetTranslation() +
		player_->GetWeapon(PlayerWeaponType::Right)->GetTranslation()) / 2.0f;
}

void PlayerCatchWeaponAttack::Update() {

	// 時間経過で処理終了
	if (lerpPos_.timer.IsReached()) {

		exitTimer_.Update();
		return;
	}

	// 座標補間更新
	{
		lerpPos_.timer.Update();
		Vector3 pos = Vector3::Lerp(lerpPos_.start, lerpPos_.target, lerpPos_.timer.easedT_);
		player_->SetTranslation(pos);
	}
}

void PlayerCatchWeaponAttack::Exit() {

	// リセット
	lerpPos_.timer.Reset();
	exitTimer_.Reset();

	// 親子付けを元に戻す
	player_->ResetWeaponTransform(PlayerWeaponType::Left);
	player_->ResetWeaponTransform(PlayerWeaponType::Right);
}

void PlayerCatchWeaponAttack::ImGui() {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);

	exitTimer_.ImGui("ExitTimer");
	lerpPos_.ImGui("LerpPos");
}

void PlayerCatchWeaponAttack::FromJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.0f);

	exitTimer_.FromJson(data.value("ExitTimer", Json()));
	lerpPos_.timer.FromJson(data.value("LerpPosTimer", Json()));
}

void PlayerCatchWeaponAttack::ToJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;

	exitTimer_.ToJson(data["ExitTimer"]);
	lerpPos_.timer.ToJson(data["LerpPosTimer"]);
}
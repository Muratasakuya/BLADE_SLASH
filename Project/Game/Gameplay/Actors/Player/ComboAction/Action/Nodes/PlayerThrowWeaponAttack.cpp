#include "PlayerThrowWeaponAttack.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerThrowWeaponAttack classMethods
//============================================================================

void PlayerThrowWeaponAttack::SetProgress([[maybe_unused]] float progress) {
}

bool PlayerThrowWeaponAttack::IsFinished() const {

	// アニメーション終了次第終了
	return player_->IsAnimationFinished();
}

float PlayerThrowWeaponAttack::GetTotalTime() const {

	// 座標補間時間がそのまま最大時間
	return player_->GetAnimationDuration("player_attack_3rd_Throw") + nextAnimDuration_;
}

void PlayerThrowWeaponAttack::Enter() {

	// リセット
	lerpPos_.timer.Reset();
	lerpRotate_.timer.Reset();

	// アニメーションの再生
	player_->SetNextAnimation("player_attack_3rd_Throw", false, nextAnimDuration_);

	//============================================================================
	// 移動開始位置、目標位置の設定
	//============================================================================
	{
		// 敵が攻撃可能範囲にいるかチェック
		isInRange_ = areaChecker_->IsInRange(AreaReactionType::LerpPos);
		Vector3 direction = Math::GetDirection3D(*player_, *attackTarget_);
		// 補間先がいなければ正面向き
		if (!isInRange_) {

			direction = player_->GetTransform().GetForward();
			direction.y = 0.0f;
			direction = direction.Normalize();
		}
		// 補間座標を設定
		lerpPos_.start = player_->GetTranslation();
		lerpPos_.target = lerpPos_.start + direction * backMoveDistance_;
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

void PlayerThrowWeaponAttack::Update() {

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

	// キーイベント処理
	UpdateAnimKeyEvent();
	// 武器補間処理
	LerpWeapon(PlayerWeaponType::Left);
	LerpWeapon(PlayerWeaponType::Right);
}

void PlayerThrowWeaponAttack::UpdateAnimKeyEvent() {

	// 左手を離した瞬間
	if (player_->IsEventKey("OutSword", 0)) {

		// 補間開始
		StartMoveWeapon(PlayerWeaponType::Left);
	}

	// 右手を離した瞬間
	if (player_->IsEventKey("OutSword", 1)) {

		// 補間開始
		StartMoveWeapon(PlayerWeaponType::Right);
	}
}

void PlayerThrowWeaponAttack::StartMoveWeapon(PlayerWeaponType type) {

	// α値を下げる
	player_->GetWeapon(type)->SetAlpha(0.5f);
	// 剣の親子付けを解除する
	player_->GetWeapon(type)->SetParent(Transform3D(), true);

	// 武器パラメータの参照
	WeaponParam& weaponParam = weaponParams_[static_cast<uint32_t>(type)];

	// この時点のワールド座標を補間開始座標にする
	weaponParam.startPos = player_->GetWeapon(type)->GetTransform().GetWorldPos();
	weaponParam.startPos.y = weaponPosY_;
	// 開始座標として設定しておく
	player_->GetWeapon(type)->SetTranslation(weaponParam.startPos);

	// 目標座標を設定する
	if (isInRange_) {

		// Y軸回転オフセットをかける
		Vector3 rotated = RotateYOffset(player_->GetTransform().GetForward(),
			weaponParam.offsetRotationY).Normalize();

		// 敵を中心に一定距離だけオフセット
		weaponParam.targetPos = attackTarget_->GetTranslation() + rotated * bossEnemyDistance_;
		weaponParam.targetPos.y = weaponPosY_;
	} else {

		// 前方ベクトル
		Vector3 forward = player_->GetTransform().GetForward();
		forward.y = 0.0f;
		forward = forward.Normalize();

		// Y軸回転オフセットをかける
		Vector3 rotated = RotateYOffset(forward, weaponParam.offsetRotationY / 6.0f);

		// 目標座標
		weaponParam.targetPos = player_->GetTranslation() + rotated * weaponParam.moveValue;
		weaponParam.targetPos.y = weaponPosY_;
	}

	// 補間開始
	weaponParam.isMoveStart = true;
	weaponParam.moveTimer = weaponMoveTimer_;
}

void PlayerThrowWeaponAttack::LerpWeapon(PlayerWeaponType type) {

	// 武器パラメータの参照
	WeaponParam& weaponParam = weaponParams_[static_cast<uint32_t>(type)];

	// 補間開始合図まで処理しない
	if (!weaponParam.isMoveStart) {
		return;
	}

	// 時間を進める
	weaponParam.moveTimer.Update();

	// 補間座標を剣に設定
	Vector3 pos = Vector3::Lerp(weaponParam.startPos, weaponParam.targetPos, weaponParam.moveTimer.easedT_);
	player_->GetWeapon(type)->SetTranslation(pos);

	// 補間終了後座標をとどめる
	if (weaponParam.moveTimer.IsReached()) {

		player_->GetWeapon(type)->SetTranslation(weaponParam.targetPos);
	}

	switch (type) {
	case PlayerWeaponType::Left:

		// 回転
		weaponParam.rotation.z = pi / 2.0f;
		weaponParam.rotation.y += weaponParam.rotateSpeed;
		player_->GetWeapon(type)->SetRotation(Quaternion::Normalize(
			Quaternion::EulerToQuaternion(weaponParam.rotation)));
		break;
	case PlayerWeaponType::Right:

		// 回転
		weaponParam.rotation.x = pi / 2.0f;
		weaponParam.rotation.y += weaponParam.rotateSpeed;
		player_->GetWeapon(type)->SetRotation(Quaternion::Normalize(
			Quaternion::EulerToQuaternion(weaponParam.rotation)));
		break;
	}
}

Vector3 PlayerThrowWeaponAttack::RotateYOffset(
	const Vector3& direction, float offsetRotationY) {

	float cos = std::cos(offsetRotationY);
	float sin = std::sin(offsetRotationY);

	Vector3 rotated{};
	rotated.x = direction.x * cos - direction.z * sin;
	rotated.y = 0.0f;
	rotated.z = direction.x * sin + direction.z * cos;

	return rotated;
}

void PlayerThrowWeaponAttack::Exit() {

	// リセット
	lerpPos_.timer.Reset();
	lerpRotate_.timer.Reset();
	// 武器パラメータリセット
	for (auto& param : weaponParams_) {

		param.isMoveStart = false;
		param.moveTimer.Reset();
		param.rotation.Init();
	}
}

void PlayerThrowWeaponAttack::ImGui() {

	ImGui::Text(std::format("isInRange: {}", isInRange_).c_str());

	ImGui::SeparatorText("Player Param");

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("backMoveDistance", &backMoveDistance_, 0.01f);

	lerpPos_.ImGui("LerpPos");
	lerpRotate_.ImGui("LerpRotate");

	ImGui::SeparatorText("Weapon Params");

	ImGui::DragFloat("bossEnemyDistance", &bossEnemyDistance_, 0.1f);
	ImGui::DragFloat("weaponPosY", &weaponPosY_, 0.1f);
	weaponMoveTimer_.ImGui("WeaponMoveTimer");

	uint32_t index = 0;
	for (auto& param : weaponParams_) {

		ImGui::PushID(index);

		PlayerWeaponType type = EnumAdapter<PlayerWeaponType>::GetValue(index);
		ImGui::SeparatorText(EnumAdapter<PlayerWeaponType>::ToString(type));

		ImGui::Text(std::format("isMoveStart: {}", param.isMoveStart).c_str());
		ImGui::DragFloat("moveValue", &param.moveValue, 0.1f);
		ImGui::DragFloat("rotateSpeed", &param.rotateSpeed, 0.01f);
		ImGui::DragFloat("offsetRotationY", &param.offsetRotationY, 0.01f);

		ImGui::PopID();

		// 飛ばされる予定の剣の位置
		Vector3 rotated = RotateYOffset(player_->GetTransform().GetForward(), param.offsetRotationY);
		Vector3 target = attackTarget_->GetTranslation() + rotated * bossEnemyDistance_;
		target.y = weaponPosY_;

		LineRenderer::GetInstance()->DrawSphere(6, 8.0f, target, Color::Cyan());
		++index;
	}
	{
		Vector3 pos = Vector3::Lerp(lerpPos_.start, lerpPos_.target, lerpPos_.timer.easedT_);
		LineRenderer::GetInstance()->DrawSphere(6, 8.0f, pos, Color::Yellow());
	}
}

void PlayerThrowWeaponAttack::FromJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.2f);
	backMoveDistance_ = data.value("backMoveDistance_", 0.2f);
	bossEnemyDistance_ = data.value("bossEnemyDistance_", 5.0f);
	weaponPosY_ = data.value("weaponPosY_", 1.0f);

	lerpPos_.timer.FromJson(data.value("LerpPosTimer", Json()));
	lerpRotate_.timer.FromJson(data.value("LerpRotateTimer", Json()));
	weaponMoveTimer_.FromJson(data.value("WeaponMoveTimer", Json()));
	if (data.contains("Weapon")) {

		uint32_t index = 0;
		for (auto& param : weaponParams_) {

			PlayerWeaponType type = EnumAdapter<PlayerWeaponType>::GetValue(index);
			const auto& key = SakuEngine::EnumAdapter<PlayerWeaponType>::ToString(type);
			param.moveValue = data["Weapon"][key]["moveValue"];
			param.rotateSpeed = data["Weapon"][key].value("rotateSpeed", 1.0f);
			param.offsetRotationY = data["Weapon"][key]["offsetRotationY"];
			++index;
		}
	}
}

void PlayerThrowWeaponAttack::ToJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["backMoveDistance_"] = backMoveDistance_;
	data["bossEnemyDistance_"] = bossEnemyDistance_;
	data["weaponPosY_"] = weaponPosY_;

	lerpPos_.timer.ToJson(data["LerpPosTimer"]);
	lerpRotate_.timer.ToJson(data["LerpRotateTimer"]);
	weaponMoveTimer_.ToJson(data["WeaponMoveTimer"]);
	for (uint32_t index = 0; index < kWeaponCount; ++index) {

		PlayerWeaponType type = EnumAdapter<PlayerWeaponType>::GetValue(index);
		const auto& key = SakuEngine::EnumAdapter<PlayerWeaponType>::ToString(type);
		data["Weapon"][key]["moveValue"] = weaponParams_[index].moveValue;
		data["Weapon"][key]["rotateSpeed"] = weaponParams_[index].rotateSpeed;
		data["Weapon"][key]["offsetRotationY"] = weaponParams_[index].offsetRotationY;
	}
}
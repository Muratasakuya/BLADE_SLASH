#include "PlayerSkilAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	PlayerSkilAttackState classMethods
//============================================================================

void PlayerSkilAttackState::CreateEffect() {

	// キーフレームオブジェクトの生成
	moveKeyframeObject_ = std::make_unique<SakuEngine::KeyframeObject3D>();
	moveKeyframeObject_->Init("playerSkilMoveKey");
	jumpKeyframeObject_ = std::make_unique<SakuEngine::KeyframeObject3D>();
	jumpKeyframeObject_->Init("playerSkilJumpKey");

	// 空の親トランスフォームの生成
	SakuEngine::ObjectManager* objectManager = SakuEngine::ObjectManager::GetInstance();
	uint32_t moveFrontID = objectManager->BuildEmptyobject("playerSkilMoveFrontTransform", "Player");
	moveFrontTag_ = objectManager->GetData<SakuEngine::ObjectTag>(moveFrontID);
	// トランスフォームを追加
	moveFrontTransform_ = objectManager->GetObjectPoolManager()->AddData<SakuEngine::Transform3D>(moveFrontID);
	moveFrontTransform_->Init();
	moveFrontTransform_->SetInstancingName(moveFrontTag_->name);
	// プレイヤーを親に設定
	moveFrontTransform_->parent = &player_->GetTransform();

	// 敵のトランスフォーム補正用の生成
	uint32_t fixedEnemyID = objectManager->BuildEmptyobject("fixedEnemyTransform", "BossEnemy");
	fixedEnemyTag_ = objectManager->GetData<SakuEngine::ObjectTag>(fixedEnemyID);
	// トランスフォームを追加
	fixedEnemyTransform_ = objectManager->GetObjectPoolManager()->AddData<SakuEngine::Transform3D>(fixedEnemyID);
	fixedEnemyTransform_->Init();
	fixedEnemyTransform_->isCompulsion_ = true;
	fixedEnemyTransform_->SetInstancingName(fixedEnemyTag_->name);

	// 残像表現エフェクト作成
	afterImageEffect_ = std::make_unique<PlayerAfterImageEffect>();
	afterImageEffect_->Init("playerAttackSkilMove");

	// 移動エフェクト作成
	moveAtackEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	moveAtackEffect_->Init("skilMoveAtackEffect", "PlayerEffect");
	moveAtackEffect_->LoadJson("GameEffectGroup/Player/skilMoveAtackEffect.json");
	// 地割れエフェクト作成
	groundCrackEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	groundCrackEffect_->Init("skilGroundCrack", "PlayerEffect");
	groundCrackEffect_->LoadJson("GameEffectGroup/Player/groundSkilCrackEffect.json");
	groundCrackEmitted_ = false;
}

void PlayerSkilAttackState::Enter() {

	// 最初のアニメーションに設定
	player_->SetNextAnimation("player_skilAttack_1st", false, nextAnimDuration_);

	// 状態設定
	currentState_ = State::MoveAttack;
	canExit_ = false;
	exitTimer_ = 0.0f;

	// 敵が攻撃可能範囲にいるかチェックして目標を設定
	SetTargetByRange(*moveKeyframeObject_, "playerSkilMove");

	// キーフレーム補間開始
	moveKeyframeObject_->StartLerp();

	// 移動前座標を初期化
	preMovePos_ = player_->GetTranslation();

	// 残像表現エフェクト開始
	std::vector<SakuEngine::GameObject3D*> objects = {
		player_,
		player_->GetWeapon(PlayerWeaponType::Left),
		player_->GetWeapon(PlayerWeaponType::Right)
	};
	afterImageEffect_->Start(objects);

	// カメラアニメーション開始
	followCamera_->StartPlayerActionAnim("playerSkilMove");
}

void PlayerSkilAttackState::Update() {

	// 状態ごとの更新
	switch (currentState_) {
	case PlayerSkilAttackState::State::MoveAttack:

		// 移動攻撃更新
		UpdateMoveAttack();
		break;
	case PlayerSkilAttackState::State::JumpAttack:

		// ジャンプ攻撃更新
		UpdateJumpAttack();
		break;
	}
}

void PlayerSkilAttackState::UpdateMoveAttack() {

	// トランスフォーム補間更新
	moveKeyframeObject_->SelfUpdate();

	// 補間された回転、座標をプレイヤーに適用
	SakuEngine::Vector3 currentTranslation = moveKeyframeObject_->GetCurrentTransform().translation;
	player_->SetTranslation(currentTranslation);
	// 回転は次の移動位置の方向を向くようにする
	// 方向
	SakuEngine::Vector3 direction = SakuEngine::Vector3(currentTranslation - preMovePos_).Normalize();
	SakuEngine::Quaternion rotation = SakuEngine::Quaternion::LookRotation(direction, rotationAxis_);
	player_->SetRotation(SakuEngine::Quaternion::Normalize(rotation));

	// 移動座標を更新する
	preMovePos_ = currentTranslation;

	// 進捗をチェックしてヒットストップを発生させる
	if (!moveAttackHitstop_.isStarted &&
		moveAttackHitstop_.startProgress <= moveKeyframeObject_->GetProgress()) {

		// ヒットストップ発生
		moveAttackHitstop_.isStarted = true;
		moveAttackHitstop_.hitStop.Start();
	}

	// 次のキーに到達するたんびにエフェクトを発生させる
	if (moveKeyframeObject_->IsNextKeyReached()) {

		// 向きを基に回転を作成
		SakuEngine::Quaternion effectRotation = SakuEngine::Quaternion::LookRotation(direction, rotationAxis_);
		moveAtackEffect_->SetParentRotation("playerSkilMoveEffect",
			SakuEngine::Quaternion::Normalize(effectRotation), ParticleUpdateModuleID::Rotation);

		// キーの位置からエフェクト発生
		SakuEngine::Vector3 translation = moveKeyframeObject_->GetIndexKeyTransform(
			moveKeyframeObject_->GetNextKeyIndex() - 1).translation;
		moveAtackEffect_->Emit(translation);
	}

	// 補間処理終了後状態を終了
	if (!moveKeyframeObject_->IsUpdating()) {

		// 状態を進める
		currentState_ = State::JumpAttack;

		// 残像表現エフェクト終了
		std::vector<SakuEngine::GameObject3D*> objects = {
			player_,
			player_->GetWeapon(PlayerWeaponType::Left),
			player_->GetWeapon(PlayerWeaponType::Right)
		};
		afterImageEffect_->End(objects);

		// ジャンプ攻撃アニメーションに設定
		player_->SetNextAnimation("player_skilAttack_2nd", false, nextJumpAnimDuration_);

		// 敵が攻撃可能範囲にいるかチェックして目標への回転を取得
		if (CheckInRange(attackPosLerpCircleRange_, SakuEngine::Math::GetDistance3D(*player_, *bossEnemy_, true, true))) {

			// 範囲内なので敵の方向を向く回転を設定する
			SakuEngine::Vector3 toEnemyDirection = SakuEngine::Math::GetDirection3D(*player_, *bossEnemy_);
			targetRotation_ = SakuEngine::Quaternion::LookRotation(toEnemyDirection, rotationAxis_);
		} else {

			// 範囲外なので前方を向く回転を設定する
			// 移動後後ろ向いているのでGetBackで前方を取得
			SakuEngine::Vector3 forward = player_->GetTransform().GetBack();
			targetRotation_ = SakuEngine::Quaternion::LookRotation(forward, rotationAxis_);
		}

		// Enterした瞬間の回転を設定
		player_->SetRotation(SakuEngine::Quaternion::Normalize(targetRotation_));
		// 行列を更新
		player_->UpdateMatrix();
		moveFrontTransform_->UpdateMatrix();

		// この時点の位置でまた範囲をチェックする
		// 敵が攻撃可能範囲にいるかチェックして目標を設定
		SetTargetByRange(*jumpKeyframeObject_, "playerSkilJump");

		// ジャンプキーフレーム補間開始
		jumpKeyframeObject_->UpdateKey(true);
		jumpKeyframeObject_->StartLerp();

		// カメラアニメーション開始
		followCamera_->StartPlayerActionAnim("playerSkilJump");
	}
}

void PlayerSkilAttackState::UpdateJumpAttack() {

	// トランスフォーム補間更新
	jumpKeyframeObject_->SelfUpdate();

	// 進捗をチェックしてヒットストップを発生させる
	if (!jumpAttackHitstop_.isStarted &&
		jumpAttackHitstop_.startProgress <= jumpKeyframeObject_->GetProgress()) {

		// ヒットストップ発生
		jumpAttackHitstop_.isStarted = true;
		jumpAttackHitstop_.hitStop.Start();
	}
	// 進捗をチェックしてジャンプエフェクトを発生させる
	if (!jumpMoveEffectEmitted_ &&
		jumpEffectEmitProgress_ <= jumpKeyframeObject_->GetProgress()) {

		// 発生済みにする
		jumpMoveEffectEmitted_ = true;

		// 目標へ向けた回転
		SakuEngine::Vector3 direction{};
		const SakuEngine::Vector3 playerPos = player_->GetTranslation();
		if (isInRange_) {

			// 敵の方向
			direction = SakuEngine::Vector3(bossEnemy_->GetTranslation() - playerPos).Normalize();
		} else {

			// 前方の方向
			SakuEngine::Vector3 targetTranslation = jumpKeyframeObject_->GetLastKeyTransform().translation;
			direction = SakuEngine::Vector3(targetTranslation - playerPos).Normalize();
		}
		// 向きを基に回転を作成
		SakuEngine::Quaternion effectRotation = SakuEngine::Quaternion::LookRotation(direction, rotationAxis_);
		moveAtackEffect_->SetParentRotation("playerSkilMoveEffect",
			SakuEngine::Quaternion::Normalize(effectRotation), ParticleUpdateModuleID::Rotation);
		// プレイヤーの右手からエフェクト発生
		moveAtackEffect_->Emit(player_->GetWeapon(PlayerWeaponType::Right)->GetTransform().GetWorldPos());
	}

	// 補間処理終了後状態を終了
	if (!jumpKeyframeObject_->IsUpdating()) {

		// 発生していないときのみ
		if (!groundCrackEmitted_) {

			// 地割れエフェクトの発生
			// Y座標は固定
			SakuEngine::Vector3 emitPos = player_->GetTranslation();
			// 地面に隠れない位置に調整
			emitPos.y = 1.0f;
			groundCrackEffect_->Emit(emitPos);
			// 発生済みにする
			groundCrackEmitted_ = true;
		}

		// 経過時間更新
		exitTimer_ += SakuEngine::GameTimer::GetDeltaTime();
		// 時間経過後に状態終了可能にする
		if (exitTime_ <= exitTimer_) {

			canExit_ = true;
		}
	} else {

		// 補間された回転、座標をプレイヤーに適用
		SakuEngine::Vector3 currentTranslation = jumpKeyframeObject_->GetCurrentTransform().translation;
		player_->SetTranslation(currentTranslation);
	}
}

void PlayerSkilAttackState::SetTargetByRange(SakuEngine::KeyframeObject3D& keyObject, const std::string& cameraKeyName) {

	// 敵が攻撃可能範囲にいるかチェック
	isInRange_ = CheckInRange(attackPosLerpCircleRange_, SakuEngine::Math::GetDistance3D(*player_, *bossEnemy_, true, true));
	if (isInRange_) {

		// 位置補正用トランスフォームを敵の位置に設定
		fixedEnemyTransform_->translation = bossEnemy_->GetTranslation();
		fixedEnemyTransform_->translation.y = 0.0f;
		fixedEnemyTransform_->rotation = bossEnemy_->GetRotation();
		// 行列を更新
		fixedEnemyTransform_->UpdateMatrix();

		// 範囲内なので敵を親の位置として設定する
		keyObject.SetParent(fixedEnemyTag_->name, *fixedEnemyTransform_);
		followCamera_->SetEditorParentTransform(cameraKeyName, *fixedEnemyTransform_);
	} else {

		// 範囲外なので空の親トランスフォームを親の位置として設定する
		keyObject.SetParent(moveFrontTag_->name, *moveFrontTransform_);
		followCamera_->SetEditorParentTransform(cameraKeyName, *moveFrontTransform_);
	}
}

void PlayerSkilAttackState::BeginUpdateAlways() {

	// キーフレームオブジェクトの更新
	moveFrontTransform_->UpdateMatrix();
	moveKeyframeObject_->UpdateKey();
	jumpKeyframeObject_->UpdateKey();
}

void PlayerSkilAttackState::UpdateAlways() {

	// ヒットストップの更新
	moveAttackHitstop_.hitStop.Update();
	jumpAttackHitstop_.hitStop.Update();

	// エフェクトの更新
	// 移動攻撃エフェクト
	moveAtackEffect_->Update();
	// 地割れ
	groundCrackEffect_->Update();
}

void PlayerSkilAttackState::Exit() {

	// リセット
	canExit_ = false;
	exitTimer_ = 0.0f;
	// 補間を確実に終了させる
	moveKeyframeObject_->Reset();
	jumpKeyframeObject_->Reset();
	moveAttackHitstop_.isStarted = false;
	jumpAttackHitstop_.isStarted = false;
	jumpMoveEffectEmitted_ = false;
	groundCrackEmitted_ = false;

	// カメラアニメーション終了
	followCamera_->EndPlayerActionAnim(true);

	// 初期Y座標に戻す
	SakuEngine::Vector3 currentPos = player_->GetTranslation();
	currentPos.y = player_->GetInitTransform().translation.y;
	player_->SetTranslation(currentPos);

	// X軸回転を0.0fに戻す
	SakuEngine::Quaternion currentRotation = player_->GetRotation();
	// X軸まわりのツイスト回転を取得
	SakuEngine::Quaternion twistX = SakuEngine::Quaternion::ExtractTwistX(currentRotation);
	SakuEngine::Quaternion twistInverse = SakuEngine::Quaternion::Inverse(twistX);
	// X軸まわりのツイストを除去した回転
	player_->SetRotation(SakuEngine::Quaternion::Normalize(SakuEngine::Quaternion::Multiply(currentRotation, twistInverse)));

	// 残像表現エフェクト終了
	std::vector<SakuEngine::GameObject3D*> objects = {
		player_,
		player_->GetWeapon(PlayerWeaponType::Left),
		player_->GetWeapon(PlayerWeaponType::Right)
	};
	afterImageEffect_->End(objects);
}

void PlayerSkilAttackState::ImGui() {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());

	ImGui::Separator();

	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("nextJumpAnimDuration", &nextJumpAnimDuration_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("jumpEffectEmitProgres", &jumpEffectEmitProgress_, 0.01f);

	ImGui::DragFloat3("rotationAxis", &rotationAxis_.x, 0.01f);
	PlayerBaseAttackState::ImGui();

	ImGui::SeparatorText("MoveFront Effect");

	afterImageEffect_->ImGui();

	ImGui::SeparatorText("MoveFront Transform");

	moveFrontTransform_->ImGui(200.0f);

	SakuEngine::LineRenderer::GetInstance()->DrawOBB(moveFrontTransform_->GetWorldPos(),
		moveFrontTransform_->scale, moveFrontTransform_->rotation, SakuEngine::Color::Cyan());

	ImGui::SeparatorText("KeyframeObject3D");

	if (ImGui::CollapsingHeader("MoveKeyframeObject")) {

		moveKeyframeObject_->ImGui();
	}
	if (ImGui::CollapsingHeader("JumpKeyframeObject")) {

		jumpKeyframeObject_->ImGui();
	}

	ImGui::SeparatorText("Hitstop");

	if (ImGui::CollapsingHeader("MoveAttackHitstop")) {

		ImGui::DragFloat("startProgress", &moveAttackHitstop_.startProgress, 0.001f);
		moveAttackHitstop_.hitStop.ImGui("MoveAttack", false);
	}
	if (ImGui::CollapsingHeader("JumpAttackHitstop")) {

		ImGui::DragFloat("startProgress", &jumpAttackHitstop_.startProgress, 0.001f);
		jumpAttackHitstop_.hitStop.ImGui("JumpAttack", false);
	}
}

void PlayerSkilAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	nextJumpAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextJumpAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "exitTime_");
	jumpEffectEmitProgress_ = data.value("jumpEffectEmitProgress_", 0.5f);

	PlayerBaseAttackState::ApplyJson(data);

	rotationAxis_ = SakuEngine::Vector3::FromJson(data.value("rotationAxis_", Json()));

	moveFrontTransform_->FromJson(data.value("MoveFrontTransform", Json()));
	moveKeyframeObject_->FromJson(data.value("MoveKey", Json()));
	jumpKeyframeObject_->FromJson(data.value("JumpKey", Json()));

	moveAttackHitstop_.startProgress = data.value("MoveAttackHitstopStartProgress", 0.0f);
	moveAttackHitstop_.hitStop.FromJson(data.value("MoveAttackHitstop", Json()));
	jumpAttackHitstop_.startProgress = data.value("JumpAttackHitstopStartProgress", 0.0f);
	jumpAttackHitstop_.hitStop.FromJson(data.value("JumpAttackHitstop", Json()));
}

void PlayerSkilAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["nextJumpAnimDuration_"] = nextJumpAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;
	data["jumpEffectEmitProgress_"] = jumpEffectEmitProgress_;

	PlayerBaseAttackState::SaveJson(data);

	data["rotationAxis_"] = rotationAxis_.ToJson();

	moveFrontTransform_->ToJson(data["MoveFrontTransform"]);
	moveKeyframeObject_->ToJson(data["MoveKey"]);
	jumpKeyframeObject_->ToJson(data["JumpKey"]);

	data["MoveAttackHitstopStartProgress"] = moveAttackHitstop_.startProgress;
	moveAttackHitstop_.hitStop.ToJson(data["MoveAttackHitstop"]);
	data["JumpAttackHitstopStartProgress"] = jumpAttackHitstop_.startProgress;
	jumpAttackHitstop_.hitStop.ToJson(data["JumpAttackHitstop"]);
}
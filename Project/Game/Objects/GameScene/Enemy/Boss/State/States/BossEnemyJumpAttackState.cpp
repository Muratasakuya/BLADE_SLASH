#include "BossEnemyJumpAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Animation/LerpKeyframe.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	BossEnemyJumpAttackState classMethods
//============================================================================

void BossEnemyJumpAttackState::CreateEffect() {

	// サイズ分確保
	float offsetY = 2.0f;
	for (uint32_t i = 0; i < jumpKeyframeCount_; ++i) {

		// iでオフセット
		float posY = offsetY * static_cast<float>(i);
		jumpKeyframes_.emplace_back(SakuEngine::Vector3(0.0f, posY, 0.0f));
	}
	canExit_ = false;

	// 剣エフェクト作成
	slash_.effect = std::make_unique<SakuEngine::EffectGroup>();
	slash_.effect->Init("jumpAttackSlash", "BossEnemyEffect");
	slash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyJumpAttackEffect_0.json");

	// 親の設定
	slash_.effect->SetParent("bossSlash_2", bossEnemy_->GetTransform());
	slash_.effectNodeName = "bossSlash_2";
}

void BossEnemyJumpAttackState::Enter() {

	// 予備動作アニメーションの再生
	bossEnemy_->SetNextAnimation("bossEnemy_jumpPrepare", false, nextAnimDuration_);

	// 最初の状態で初期化
	currentState_ = State::Pre;

	// 攻撃予兆を出す
	SakuEngine::Vector3 sign = bossEnemy_->GetTranslation();
	sign.y = 2.0f;
	attackSign_->Emit(SakuEngine::Math::ProjectToScreen(sign, *followCamera_));

	// パリィ可能にする
	bossEnemy_->ResetParryTiming();
	parryParam_.continuousCount = 1;
	parryParam_.canParry = true;
}

void BossEnemyJumpAttackState::Update() {

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyJumpAttackState::State::Pre:

		// 予備動作
		UpdatePre();
		break;
	case BossEnemyJumpAttackState::State::Jump:

		// ジャンプ
		UpdateJump();
		break;
	}
}

void BossEnemyJumpAttackState::UpdatePre() {

	// 予備動作中はプレイヤーの方を向く
	LookTarget(player_->GetTranslation());

	// 予備動作が終了したらジャンプ状態へ
	if (bossEnemy_->IsAnimationFinished()) {

		// ジャンプアニメーションの再生
		bossEnemy_->SetNextAnimation("bossEnemy_jumpAttack", false, nextAnimDuration_);

		// ジャンプ状態へ
		currentState_ = State::Jump;

		// 補間座標の設定
		SetLerpTranslation();
		// 補間開始
		lerpTranslationXZ_.Start();

		// 剣エフェクト発生
		slash_.Emit(*bossEnemy_);
	}
}

void BossEnemyJumpAttackState::UpdateJump() {

	// パリィ攻撃のタイミングを伝える
	if (bossEnemy_->IsEventKey("Parry", 0)) {

		bossEnemy_->TellParryTiming();
	}

	// 座標を補間
	SakuEngine::Vector3 translation = bossEnemy_->GetTranslation();
	lerpTranslationXZ_.LerpValue(translation);

	// Y座標をジャンプ補間、Y座標のみ返す補間
	translation.y = LerpKeyframe::GetValue(jumpKeyframes_,
		EasedValue(jumpEasing_, lerpTranslationXZ_.GetProgress()), LerpKeyframe::Type::Spline).y;

	// 座標を設定
	bossEnemy_->SetTranslation(translation);

	//補間が終了したら状態を終了
	if (lerpTranslationXZ_.IsFinished()) {

		canExit_ = true;
	}
}

void BossEnemyJumpAttackState::UpdateAlways() {

	// 剣エフェクト更新
	slash_.Update(*bossEnemy_);
}

void BossEnemyJumpAttackState::Exit() {

	// リセット
	canExit_ = false;
	parryParam_.canParry = false;
	lerpTranslationXZ_.Reset();
}

void BossEnemyJumpAttackState::SetLerpTranslation() {

	// 補間座標の設定
	// 開始座標
	lerpTranslationXZ_.SetStart(bossEnemy_->GetTranslation());
	// 終了座標
	SakuEngine::Vector3 playerPos = player_->GetTranslation();
	SakuEngine::Vector3 direction = SakuEngine::Vector3(playerPos - bossEnemy_->GetTranslation()).Normalize();
	lerpTranslationXZ_.SetEnd(playerPos - direction * targetDistance_);

	for (uint32_t i = 0; i < jumpKeyframeCount_; ++i) {

		// 分割した時のt値
		float t = static_cast<float>(i) / static_cast<float>(jumpKeyframeCount_ - 1);
		SakuEngine::Vector3 translation = SakuEngine::Vector3::Lerp(lerpTranslationXZ_.GetStart(), lerpTranslationXZ_.GetEnd(), t);
		jumpKeyframes_[i].x = translation.x;
		jumpKeyframes_[i].z = translation.z;
	}
}

void BossEnemyJumpAttackState::ImGui() {

	ImGui::Text(std::format("canEixt: {}", canExit_).c_str());
	ImGui::Text("currentState: %s", SakuEngine::EnumAdapter<State>::ToString(currentState_));

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.01f);
	ImGui::DragFloat("targetDistance", &targetDistance_, 0.01f);
	ImGui::DragFloat3("slashEffectOffset", &slash_.effectOffset.x, 0.1f);

	lerpTranslationXZ_.ImGui("LerpTranslationXZ");

	ImGui::SeparatorText("Jump");

	for (uint32_t index = 0; index < jumpKeyframeCount_; ++index) {

		ImGui::PushID(index);

		SakuEngine::Vector3& keyframe = jumpKeyframes_[index];
		const std::string key = "Keyframe: " + std::to_string(index);

		ImGui::DragFloat3(key.c_str(), &keyframe.x, 0.01f);

		SakuEngine::LineRenderer::GetInstance()->DrawSphere(6, 1.0f, keyframe, SakuEngine::Color::Cyan());

		ImGui::PopID();
	}

	SakuEngine::EnumAdapter<EasingType>::Combo("JumpEasing", &jumpEasing_);

	// 補間座標の設定
	SetLerpTranslation();
}

void BossEnemyJumpAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.08f);
	rotationLerpRate_ = data.value("rotationLerpRate_", 0.08f);
	targetDistance_ = data.value("targetDistance_", 0.08f);
	lerpTranslationXZ_.FromJson(data.value("LerpTranslationXZ", Json()));

	jumpEasing_ = SakuEngine::EnumAdapter<EasingType>::FromString(data.value("jumpEasing_", "Linear")).value();
	for (uint32_t i = 0; i < jumpKeyframeCount_; ++i) {

		jumpKeyframes_[i] = SakuEngine::Vector3::FromJson(data["JumpKeyframes"][i]);
	}

	slash_.effectOffset = SakuEngine::Vector3::FromJson(data.value("slashEffectOffset_", Json()));
}

void BossEnemyJumpAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["targetDistance_"] = targetDistance_;
	lerpTranslationXZ_.ToJson(data["LerpTranslationXZ"]);

	data["jumpEasing_"] = SakuEngine::EnumAdapter<EasingType>::ToString(jumpEasing_);
	for (uint32_t i = 0; i < jumpKeyframeCount_; ++i) {

		data["JumpKeyframes"][i] = jumpKeyframes_[i].ToJson();
	}

	data["slashEffectOffset_"] = slash_.effectOffset.ToJson();
}
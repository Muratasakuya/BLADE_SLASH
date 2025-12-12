#include "PlayerParryState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/PostEffect/RadialBlurUpdater.h>

//============================================================================
//	PlayerParryState classMethods
//============================================================================

PlayerParryState::PlayerParryState() {

	isEmitedBlur_ = false;

	// エフェクト作成
	// ヒットした瞬間
	parryHitEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	parryHitEffect_->Init("parryHitEffect", "PlayerEffect");
	parryHitEffect_->LoadJson("GameEffectGroup/Player/playerParryHitEffect.json");
	// 引きずる剣先
	tipScrackEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	tipScrackEffect_->Init("parryTipScrachEffect", "PlayerEffect");
	tipScrackEffect_->LoadJson("GameEffectGroup/Player/playerParryTipScrachEffect.json");
	// 攻撃ヒットエフェクト
	hitEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	hitEffect_->Init("parryAttackHit", "PlayerEffect");
	hitEffect_->LoadJson("GameEffectGroup/Player/playerHitEffect_0.json");
}

void PlayerParryState::Enter(Player& player) {

	// アニメーション再生
	player.SetNextAnimation("player_parry", false, nextAnimDuration_);

	// 座標、向きを計算
	SakuEngine::Vector3 direction = SetLerpValue(startPos_, targetPos_,
		player, parryLerp_.moveDistance, true);
	direction.y = 0.0f;
	direction = direction.Normalize();

	// 敵の方向を向かせる
	player.SetRotation(SakuEngine::Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up)));
	// 左手の武器を反転
	player.SetReverseWeapon(true, PlayerWeaponType::Left);

	// ヒットストップ開始
	SakuEngine::GameTimer::StartHitStop(deltaWaitTime_, 0.0f);

	// パリィ用のカメラアニメーションを設定
	followCamera_->StartPlayerActionAnim(PlayerState::Parry);

	// 剣先の引っかきエフェクトを発生させる
	tipScrackEffect_->Emit(player.GetWeapon(PlayerWeaponType::Left)->GetTipTranslation());

	// HUDの入力示唆を消させる
	player.GetHUD()->EndInputSuggest();

	canExit_ = false;
	isEmitedBlur_ = false;
	request_ = std::nullopt;
	parryLerp_.isFinised = false;
	attackLerp_.isFinised = false;
	deltaWaitTimer_ = 0.0f;
}

void PlayerParryState::Update(Player& player) {

	// deltaTimeの管理時間を更新
	UpdateDeltaWaitTime(player);

	// 入力状態を確認
	CheckInput();

	// 座標補間を更新
	UpdateLerpTranslation(player);

	// animationの更新
	UpdateAnimation(player);
}

void PlayerParryState::UpdateAlways(Player& player) {

	// エフェクトの更新
	parryHitEffect_->Update();
	hitEffect_->Update();

	// 剣先の座標を常に更新
	tipScrackEffect_->SetWorldPos(player.GetWeapon(PlayerWeaponType::Left)->GetTipTranslation());
	tipScrackEffect_->Update();
}

void PlayerParryState::UpdateDeltaWaitTime(const Player& player) {

	// 時間経過を進める
	deltaWaitTimer_ += SakuEngine::GameTimer::GetDeltaTime();
	// 時間経過が過ぎたら元に戻させる
	if (deltaWaitTime_ < deltaWaitTimer_) {

		// まだブラーが発生していなければ発生させる
		if (!isEmitedBlur_) {

			// 左手にエフェクトを発生させる
			// 発生座標
			SakuEngine::Vector3 effectPos = player.GetJointWorldPos("leftHand");
			effectPos.y = parryHitEffectPosY_;
			parryHitEffect_->Emit(effectPos);

			// ブラー手の位置に発生させる
			postProcess_->Start(PostProcessType::RadialBlur);
			RadialBlurUpdater* blur = postProcess_->GetUpdater<RadialBlurUpdater>(
				PostProcessType::RadialBlur);
			// 自動で元の値に戻すように設定
			blur->StartState();
			blur->SetBlurType(RadialBlurType::Parry);
			blur->SetIsAutoReturn(true);

			// 腕の入りをスクリーン座標に変換して0.0f~1.0fの正規化する
			SakuEngine::Vector2 screenPos = SakuEngine::Math::ProjectToScreen(
				player.GetWeapon(PlayerWeaponType::Left)->GetTransform().GetWorldPos(), *followCamera_).Normalize();
			blur->SetBlurCenter(screenPos);

			// 発生済み
			isEmitedBlur_ = true;
		}
	}
}

void PlayerParryState::UpdateLerpTranslation(Player& player) {

	if (parryLerp_.isFinised) {
		return;
	}

	// 座標を補間
	SakuEngine::Vector3 translation = GetLerpTranslation(parryLerp_);

	// 座標を設定
	player.SetTranslation(translation);

	if (parryLerp_.isFinised) {

		// 攻撃入力がされていなければ滑らかに元のカメラに戻るようにする
		if (!request_.has_value()) {

			followCamera_->WarmStart();
		}

		// 補間終了後剣先エフェクトを停止させる
		tipScrackEffect_->Stop();
	}
}

void PlayerParryState::CheckInput() {

	// 座標補間が終了したら入力を受け付けない
	if (parryLerp_.isFinised) {
		return;
	}

	// deltaTimeが元に戻った後どうするかを入力確認
	// 攻撃を押していたらanimationが終了した後攻撃に移る
	if (allowAttack_ && inputMapper_->IsTriggered(PlayerInputAction::Attack)) {

		// 攻撃可能なら攻撃をリクエストする
		request_ = RequestState::PlayAnimation;
	}
}

void PlayerParryState::UpdateAnimation(Player& player) {

	// 座標補間が終了するまでなにもしない
	if (!parryLerp_.isFinised) {
		return;
	}

	// 攻撃ボタンが押されていなければ状態を終了する
	if (!request_.has_value()) {

		canExit_ = true;
		// 元に戻す
		player.SetReverseWeapon(false, PlayerWeaponType::Left);
		return;
	}
	switch (request_.value()) {
	case RequestState::PlayAnimation: {

		// 攻撃アニメーション再生
		player.SetNextAnimation("player_stunAttack", false, nextAnimDuration_);

		// 補間先の座標を再設定する
		SetLerpValue(startPos_, targetPos_, player, attackLerp_.moveDistance, false);
		request_ = RequestState::AttackAnimation;

		// カメラアニメーションを終了させる
		followCamera_->EndPlayerActionAnim(true);
		// パリィ攻撃は4段目の攻撃と同じ
		followCamera_->StartPlayerActionAnim(PlayerState::Attack_4th);

		// 元に戻す
		player.SetReverseWeapon(false, PlayerWeaponType::Left);
		break;
	}
	case RequestState::AttackAnimation: {

		// 座標補間を行う
		SakuEngine::Vector3 translation = GetLerpTranslation(attackLerp_);

		// 座標を設定
		player.SetTranslation(translation);

		// 補間が終了したら状態を終了する
		if (attackLerp_.isFinised) {

			request_ = std::nullopt;
			
			// 攻撃ヒットエフェクトを発生させる
			SakuEngine::Vector3 hitEffectPos = targetPos_;
			hitEffectPos.y = hitEffectOffsetY_;
			hitEffect_->Emit(hitEffectPos);
		}
		break;
	}
	}
}

SakuEngine::Vector3 PlayerParryState::GetLerpTranslation(LerpParameter& lerp) {

	// 時間を進める
	lerp.timer += SakuEngine::GameTimer::GetScaledDeltaTime();
	float lerpT = std::clamp(lerp.timer / lerp.time, 0.0f, 1.0f);
	lerpT = EasedValue(lerp.easingType, lerpT);

	// 座標を補間
	SakuEngine::Vector3 translation = SakuEngine::Vector3::Lerp(startPos_, targetPos_, lerpT);

	if (lerp.time < lerp.timer) {

		lerp.isFinised = true;
	}
	return translation;
}

SakuEngine::Vector3 PlayerParryState::SetLerpValue(SakuEngine::Vector3& start, SakuEngine::Vector3& target,
	const Player& player, float moveDistance, bool isPlayerBase) {

	SakuEngine::Vector3 playerPos = player.GetTranslation();
	playerPos.y = 0.0f;
	SakuEngine::Vector3 enemyPos = bossEnemy_->GetTranslation();
	enemyPos.y = 0.0f;
	// 向き
	SakuEngine::Vector3 direction = enemyPos - playerPos;
	direction = direction.Normalize();

	// 補間座標を設定する
	start = playerPos;
	if (isPlayerBase) {

		target = playerPos + direction * moveDistance;
	} else {

		target = enemyPos + direction * moveDistance;
	}
	return direction;
}

void PlayerParryState::Exit([[maybe_unused]] Player& player) {

	// カメラアニメーションを終了させる
	followCamera_->EndPlayerActionAnim(true);

	// リセット
	request_ = std::nullopt;
	deltaWaitTimer_ = 0.0f;
	parryLerp_.timer = 0.0f;
	attackLerp_.timer = 0.0f;
	parryLerp_.isFinised = false;
	attackLerp_.isFinised = false;
	canExit_ = false;
	allowAttack_ = false;
	isEmitedBlur_ = false;
}

void PlayerParryState::ImGui(const Player& player) {

	ImGui::Text(std::format("allowAttack: {}", allowAttack_).c_str());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("deltaWaitTime", &deltaWaitTime_, 0.01f);
	ImGui::DragFloat("deltaLerpSpeed_", &deltaLerpSpeed_, 0.01f);
	ImGui::DragFloat("cameraLookRate", &cameraLookRate_, 0.01f);
	ImGui::DragFloat("parryHitEffectPosY", &parryHitEffectPosY_, 0.01f);
	ImGui::DragFloat("hitEffectOffsetY", &hitEffectOffsetY_, 0.01f);

	SakuEngine::ImGuiHelper::ValueText<SakuEngine::Vector3>("stratPos", startPos_);
	SakuEngine::ImGuiHelper::ValueText<SakuEngine::Vector3>("targetPos", targetPos_);

	SakuEngine::LineRenderer* lineRenderer = SakuEngine::LineRenderer::GetInstance();

	ImGui::SeparatorText("Parry: RED");

	ImGui::DragFloat("time##Parry", &parryLerp_.time, 0.01f);
	ImGui::DragFloat("moveDistance##Parry", &parryLerp_.moveDistance, 0.1f);
	Easing::SelectEasingType(parryLerp_.easingType, "Parry");

	{
		SakuEngine::Vector3 start{};
		SakuEngine::Vector3 target{};
		SakuEngine::Vector3 translation = SetLerpValue(start, target,
			player, parryLerp_.moveDistance, true);
		start.y = 2.0f;
		target.y = 2.0f;

		lineRenderer->DrawLine3D(
			start, target, SakuEngine::Color::Red());
	}

	ImGui::SeparatorText("Attack: YELLOW");

	ImGui::DragFloat("time##Attack", &attackLerp_.time, 0.01f);
	ImGui::DragFloat("moveDistance##Attack", &attackLerp_.moveDistance, 0.1f);
	Easing::SelectEasingType(attackLerp_.easingType, "Attack");

	{
		SakuEngine::Vector3 start{};
		SakuEngine::Vector3 target{};
		SakuEngine::Vector3 translation = SetLerpValue(start, target,
			player, attackLerp_.moveDistance, false);
		start.y = 2.0f;
		target.y = 2.0f;

		lineRenderer->DrawLine3D(
			start, target, SakuEngine::Color(0.0f, 1.0f, 1.0f, 1.0f));
	}
}

void PlayerParryState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	deltaWaitTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "deltaWaitTime_");
	deltaLerpSpeed_ = data.value("deltaLerpSpeed_", 8.0f);
	cameraLookRate_ = data.value("cameraLookRate_", 1.0f);
	parryHitEffectPosY_ = data.value("parryEffectPosY_", 4.0f);
	hitEffectOffsetY_ = data.value("hitEffectOffsetY_", 4.0f);

	parryLerp_.time = SakuEngine::JsonAdapter::GetValue<float>(data, "parryLerp_.time");
	parryLerp_.moveDistance = SakuEngine::JsonAdapter::GetValue<float>(data, "parryLerp_.moveDistance");
	parryLerp_.easingType = static_cast<EasingType>(
		SakuEngine::JsonAdapter::GetValue<int>(data, "parryLerp_.easingType"));

	attackLerp_.time = SakuEngine::JsonAdapter::GetValue<float>(data, "attackLerp_.time");
	attackLerp_.moveDistance = SakuEngine::JsonAdapter::GetValue<float>(data, "attackLerp_.moveDistance");
	attackLerp_.easingType = static_cast<EasingType>(
		SakuEngine::JsonAdapter::GetValue<int>(data, "attackLerp_.easingType"));
}

void PlayerParryState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["deltaWaitTime_"] = deltaWaitTime_;
	data["deltaLerpSpeed_"] = deltaLerpSpeed_;
	data["cameraLookRate_"] = cameraLookRate_;
	data["parryEffectPosY_"] = parryHitEffectPosY_;
	data["hitEffectOffsetY_"] = hitEffectOffsetY_;

	data["parryLerp_.time"] = parryLerp_.time;
	data["parryLerp_.moveDistance"] = parryLerp_.moveDistance;
	data["parryLerp_.easingType"] = parryLerp_.easingType;

	data["attackLerp_.time"] = attackLerp_.time;
	data["attackLerp_.moveDistance"] = attackLerp_.moveDistance;
	data["attackLerp_.easingType"] = static_cast<int>(attackLerp_.easingType);
}
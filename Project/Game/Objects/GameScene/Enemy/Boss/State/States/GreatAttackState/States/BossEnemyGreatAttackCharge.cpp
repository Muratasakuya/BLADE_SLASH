#include "BossEnemyGreatAttackCharge.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Skybox.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Game/Light/GameLight.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/PostEffect/RadialBlurUpdater.h>

//============================================================================
//	BossEnemyGreatAttackCharge classMethods
//============================================================================

BossEnemyGreatAttackCharge::BossEnemyGreatAttackCharge() {

	// 初期化値
	canExit_ = false;

	// チャージ開始エフェクト
	beginChargeEffect_ = std::make_unique<EffectGroup>();
	beginChargeEffect_->Init("beginCharge", "BossEnemyEffect");
	beginChargeEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyBeginChargeEffect.json");
}

void BossEnemyGreatAttackCharge::Enter() {

	// チャージアニメーション再生
	bossEnemy_->SetNextAnimation("bossEnemy_beginChargeGreatAttack", false, nextAnimDuration_);
	emitedBeginChargeEffect_ = false;

	// 攻撃無効状態にする
	bossEnemy_->GetHUD()->SetDisable();
}

void BossEnemyGreatAttackCharge::Update() {

	// アニメーションが終了するまで処理しない
	if (!bossEnemy_->IsAnimationFinished()) {
		return;
	}
	// エフェクトの発生
	if (!emitedBeginChargeEffect_) {

		// 1回だけ発生
		beginChargeEffect_->Emit(bossEnemy_->GetTranslation() +
			(bossEnemy_->GetRotation() * beginChargeEffectOffset_));
		emitedBeginChargeEffect_ = true;
	}

	// 時間を更新
	nextTimer_.Update();

	// 時間経過後次の状態に進む
	if (nextTimer_.IsReached()) {

		canExit_ = true;
	}
}

void BossEnemyGreatAttackCharge::UpdateAlways() {

	// エフェクト更新
	beginChargeEffect_->Update();
}

void BossEnemyGreatAttackCharge::Exit() {

	// リセット
	nextTimer_.Reset();
	canExit_ = false;
	emitedBeginChargeEffect_ = false;

	// ブラーを発生させる
	PostProcessSystem* postProcess = PostProcessSystem::GetInstance();
	postProcess->Start(PostProcessType::RadialBlur);
	RadialBlurUpdater* blur = postProcess->GetUpdater<RadialBlurUpdater>(PostProcessType::RadialBlur);
	// 自動で元の値に戻すように設定
	blur->StartState();
	blur->SetBlurType(RadialBlurType::Parry);
	blur->SetIsAutoReturn(true);

	// スクリーン座標に変換して0.0f~1.0fの正規化する
	Vector2 screenPos = Math::ProjectToScreen(bossEnemy_->GetTranslation(), *followCamera_).Normalize();
	blur->SetBlurCenter(screenPos);
}

void BossEnemyGreatAttackCharge::ImGui() {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::ColorEdit4("skyboxColor", &skyboxColor_.r);
	ImGui::DragFloat3("beginChargeEffectOffset", &beginChargeEffectOffset_.x, 0.1f);
	ImGui::Separator();

	nextTimer_.ImGui("NextTimer", false);
}

void BossEnemyGreatAttackCharge::ApplyJson(const Json& data) {

	nextTimer_.FromJson(data.value("NextTimer", Json()));
	nextAnimDuration_ = data.value("nextAnimDuration", 0.0f);
	skyboxColor_ = Color::FromJson(data.value("skyboxColor", Json()));
	beginChargeEffectOffset_ = Vector3::FromJson(data.value("beginChargeEffectOffset", Json()));
}

void BossEnemyGreatAttackCharge::SaveJson(Json& data) {

	nextTimer_.ToJson(data["NextTimer"]);
	data["nextAnimDuration"] = nextAnimDuration_;
	data["skyboxColor"] = skyboxColor_.ToJson();
	data["beginChargeEffectOffset"] = beginChargeEffectOffset_.ToJson();
}
#include "BossEnemyGreatAttackInOutArea.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/BossEnemyGreatAttackState.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	BossEnemyGreatAttackInOutArea classMethods
//============================================================================

BossEnemyGreatAttackInOutArea::BossEnemyGreatAttackInOutArea() {

	// 雷攻撃エフェクト
	for (auto& effect : lightningAttackEffects_) {

		effect = std::make_unique<SakuEngine::EffectGroup>();
		effect->Init("lightningAttack", "BossEnemyEffect");
		effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightningAttackEffect.json");
	}

	// 移動キーフレーム
	attackKeyframeObject_ = std::make_unique<SakuEngine::KeyframeObject3D>();
	attackKeyframeObject_->Init("attackKeyframeObject");

	// 大技攻撃目標トランスフォーム
	grearAttackTargetTransform_ = std::make_unique<SakuEngine::Transform3D>();
	grearAttackTargetTransform_->Init();

	// 範囲斬撃エフェクト
	rangeSlashEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	rangeSlashEffect_->Init("rangeSlash", "BossEnemyEffect");
	rangeSlashEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyGreatAttackRangeSlashEffect.json");
	emitedRangeSlashEffect_ = false;
}

void BossEnemyGreatAttackInOutArea::Enter() {

	// 状態初期化
	canExit_ = false;
	currentState_ = State::Out;
	hideEnemyTimer_.Reset();
	lightningAttackTimer_.Reset();
	attackKeyframeObject_->Reset();
	endWaitTimer_.Reset();
	isPlayedAttackKeyframe_ = false;
	isPlayedGrearAttackAnim_ = false;
	emitedRangeSlashEffect_ = false;

	// ボスの表示を消す
	parentState_->StopEffects();

	// 親を設定
	attackKeyframeObject_->SetParent("", *grearAttackTargetTransform_);
}

void BossEnemyGreatAttackInOutArea::Update() {

	// 状態毎の更新
	switch (currentState_) {
	case BossEnemyGreatAttackInOutArea::State::Out:

		UpdateOut();
		break;
	case BossEnemyGreatAttackInOutArea::State::In:

		UpdateIn();
		break;
	}
}

void BossEnemyGreatAttackInOutArea::UpdateOut() {

	// 補間終了後、雷攻撃を作成
	if (hideEnemyTimer_.IsReached()) {

		// 雷攻撃時間更新
		lightningAttackTimer_.Update();

		// 時間経過後次の状態に移す
		if (lightningAttackTimer_.IsReached()) {

			// 次の状態へ
			currentState_ = State::In;

			// キーフレームアニメーション再生
			attackKeyframeObject_->StartLerp(bossEnemy_->GetTransform());

			// 攻撃予兆アニメーション再生
			bossEnemy_->SetNextAnimation("bossEnemy_slashStay", false, 0.0f);

			// ボスの表示を戻す
			parentState_->StartEffects();
		}
	} else {

		// 時間の更新
		hideEnemyTimer_.Update();

		// トリガーで雷を発生させる
		if (hideEnemyTimer_.IsReached()) {

			// 雷攻撃発生
			EmitLightningAttack();

			// 一度画面外に飛ばす
			bossEnemy_->SetTranslation(SakuEngine::Vector3(0.0f, -32, 0.0f));
		}
	}
}

void BossEnemyGreatAttackInOutArea::UpdateIn() {

	// キーフレーム補間終了後状態終了
	if (!attackKeyframeObject_->IsUpdating()) {

		endWaitTimer_.Update();
		// 待機時間終了後状態終了
		if (endWaitTimer_.IsReached()) {

			canExit_ = true;
		}
	} else {

		// キーフレーム更新
		attackKeyframeObject_->SelfUpdate();

		// 回転と座標を設定
		bossEnemy_->SetRotation(attackKeyframeObject_->GetCurrentTransform().rotation);
		bossEnemy_->SetTranslation(attackKeyframeObject_->GetCurrentTransform().translation);

		// 指定のキーインデックスを超えたら攻撃アニメーションへ移行
		if (!isPlayedAttackKeyframe_ &&
			attackKeyframeObject_->GetNextKeyIndex() == attackKeyframeIndex_) {

			bossEnemy_->SetNextAnimation("bossEnemy_stayedSlash", false, 0.0f);
			isPlayedAttackKeyframe_ = true;
		}
		// 最後の攻撃アニメーションへの遷移
		if (!isPlayedGrearAttackAnim_ &&
			attackKeyframeObject_->GetNextKeyIndex() == greatKeyframeIndex_) {

			bossEnemy_->SetNextAnimation("bossEnemy_greatAttack", false, grearAttackNextAnimTime_);
			isPlayedGrearAttackAnim_ = true;
		}

		// 範囲斬撃エフェクト発生
		if (!emitedRangeSlashEffect_ &&
			attackKeyframeObject_->GetNextKeyIndex() == rangeSlashKeyIndex_) {

			// 発生座標
			SakuEngine::Vector3 emitPos = bossEnemy_->GetTranslation();
			emitPos += attackKeyframeObject_->GetCurrentTransform().rotation * rangeSlashEffectOffset_;

			// 発生済みにする
			rangeSlashEffect_->Emit(emitPos);
			emitedRangeSlashEffect_ = true;
		}
	}
}

void BossEnemyGreatAttackInOutArea::EmitLightningAttack() {

	// n番目ごとに進める角度
	float stepAngle = SakuEngine::pi * 2.0f / static_cast<float>(lightningCount_);

	// 雷攻撃発生
	for (uint32_t i = 0; i <= lightningCount_; ++i) {

		// 現在の角度
		float angle = stepAngle * static_cast<float>(i);
		float sin = std::sin(angle);
		float cos = std::cos(angle);

		// 発生位置
		SakuEngine::Vector3 emitPos = SakuEngine::Vector3(sin * outAreaRadius_, 0.0f, cos * outAreaRadius_);

		// エフェクト発生
		lightningAttackEffects_[i]->Emit(emitPos);
	}
}

void BossEnemyGreatAttackInOutArea::UpdateAlways() {

	// エフェクト更新
	for (const auto& effect : lightningAttackEffects_) {

		effect->Update();
	}
	rangeSlashEffect_->Update();

	// キーフレーム更新
	attackKeyframeObject_->UpdateKey();

	// 状態がOutの時のみカメラのトランスフォームの値で更新させる
	if (currentState_ == State::Out) {

		// 回転と座標を設定
		grearAttackTargetTransform_->translation = followCamera_->GetTransform().translation;
		grearAttackTargetTransform_->translation.y = 0.0f;
		// 位置、回転を更新する
		SakuEngine::Vector3 forward = followCamera_->GetTransform().GetForward();
		forward.y = 0.0f;
		SakuEngine::Quaternion cameraRotation = SakuEngine::Quaternion::LookRotation(
			forward.Normalize(), SakuEngine::Vector3(0.0f, 1.0f, 0.0f));
		grearAttackTargetTransform_->rotation = SakuEngine::Quaternion::Normalize(cameraRotation);
		// 行列更新
		grearAttackTargetTransform_->UpdateMatrix();
	}
}

void BossEnemyGreatAttackInOutArea::Exit() {

	// リセット
	canExit_ = false;
	currentState_ = State::Out;
	isPlayedAttackKeyframe_ = false;
	isPlayedGrearAttackAnim_ = false;
}

void BossEnemyGreatAttackInOutArea::ImGui() {

	SakuEngine::LineRenderer* lineRenderer = SakuEngine::LineRenderer::GetInstance();

	ImGui::SeparatorText("Out");

	if (ImGui::Button("Emit Lightning")) {

		EmitLightningAttack();
	}

	ImGui::DragFloat("OutAreaRadius", &outAreaRadius_, 0.1f);
	int32_t lightningCount = static_cast<int32_t>(lightningCount_);
	ImGui::DragInt("LightningCount", &lightningCount, 1, 1);
	lightningCount_ = std::clamp(static_cast<uint32_t>(lightningCount), uint32_t(0), maxLightningCount_);

	ImGui::DragFloat("GrearAttackNextAnimTime", &grearAttackNextAnimTime_, 0.01f);

	hideEnemyTimer_.ImGui("HideEnemyTimer");
	lightningAttackTimer_.ImGui("LightningAttackTimer");

	// 円の描画
	lineRenderer->DrawCircle(lightningCount, outAreaRadius_, SakuEngine::Vector3(0.0f, 2.0f, 0.0f), SakuEngine::Color::Cyan());

	ImGui::SeparatorText("In");

	{
		int32_t attackKeyframeIndex = static_cast<int32_t>(attackKeyframeIndex_);
		ImGui::DragInt("AttackKeyframeIndex", &attackKeyframeIndex, 1, 0);
		attackKeyframeIndex_ = static_cast<uint32_t>(attackKeyframeIndex);
	}
	{
		int32_t attackKeyframeIndex = static_cast<int32_t>(greatKeyframeIndex_);
		ImGui::DragInt("GreatKeyframeIndex_", &attackKeyframeIndex, 1, 0);
		greatKeyframeIndex_ = static_cast<uint32_t>(attackKeyframeIndex);
	}
	{
		int32_t attackKeyframeIndex = static_cast<int32_t>(rangeSlashKeyIndex_);
		ImGui::DragInt("RangeSlashKeyIndex", &attackKeyframeIndex, 1, 0);
		rangeSlashKeyIndex_ = static_cast<uint32_t>(attackKeyframeIndex);
	}
	ImGui::DragFloat3("RangeSlashEffectOffset", &rangeSlashEffectOffset_.x, 0.01f);

	endWaitTimer_.ImGui("EndWaitTimer");

	if (ImGui::CollapsingHeader("AttackKeyframeObject")) {

		attackKeyframeObject_->ImGui();
	}
}

void BossEnemyGreatAttackInOutArea::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	outAreaRadius_ = data.value("outAreaRadius_", 10.0f);
	lightningCount_ = data.value("lightningCount_", 8u);
	attackKeyframeIndex_ = data.value("attackKeyframeIndex_", 8u);
	greatKeyframeIndex_ = data.value("greatKeyframeIndex_", 8u);
	lightningCount_ = std::clamp(lightningCount_, uint32_t(0), maxLightningCount_);
	grearAttackNextAnimTime_ = data.value("grearAttackNextAnimTime_", 5.0f);
	rangeSlashKeyIndex_ = data.value("rangeSlashKeyIndex_", 8u);
	rangeSlashEffectOffset_ = SakuEngine::Vector3::FromJson(data.value("rangeSlashEffectOffset_", Json()));
	hideEnemyTimer_.FromJson(data.value("hideEnemyTimer_", Json()));
	lightningAttackTimer_.FromJson(data.value("lightningAttackTimer_", Json()));
	attackKeyframeObject_->FromJson(data.value("AttackKeyframeObject", Json()));
	endWaitTimer_.FromJson(data.value("endWaitTimer_", Json()));
}

void BossEnemyGreatAttackInOutArea::SaveJson(Json& data) {

	data["outAreaRadius_"] = outAreaRadius_;
	data["lightningCount_"] = lightningCount_;
	data["attackKeyframeIndex_"] = attackKeyframeIndex_;
	data["greatKeyframeIndex_"] = greatKeyframeIndex_;
	data["grearAttackNextAnimTime_"] = grearAttackNextAnimTime_;
	data["rangeSlashKeyIndex_"] = rangeSlashKeyIndex_;
	data["rangeSlashEffectOffset_"] = rangeSlashEffectOffset_.ToJson();
	hideEnemyTimer_.ToJson(data["hideEnemyTimer_"]);
	lightningAttackTimer_.ToJson(data["lightningAttackTimer_"]);
	attackKeyframeObject_->ToJson(data["AttackKeyframeObject"]);
	endWaitTimer_.ToJson(data["endWaitTimer_"]);
}
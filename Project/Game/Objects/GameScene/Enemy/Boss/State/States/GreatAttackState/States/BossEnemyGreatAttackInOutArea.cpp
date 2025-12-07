#include "BossEnemyGreatAttackInOutArea.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	BossEnemyGreatAttackInOutArea classMethods
//============================================================================

BossEnemyGreatAttackInOutArea::BossEnemyGreatAttackInOutArea() {

	// 雷攻撃エフェクト
	for (auto& effect : lightningAttackEffects_) {

		effect = std::make_unique<EffectGroup>();
		effect->Init("lightningAttack", "BossEnemyEffect");
		effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightningAttackEffect.json");
	}

	// 移動キーフレーム
	attackKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	attackKeyframeObject_->Init("attackKeyframeObject");
}

void BossEnemyGreatAttackInOutArea::Enter() {

	// 状態初期化
	canExit_ = false;
	currentState_ = State::Out;
	hideEnemyTimer_.Reset();
	lightningAttackTimer_.Reset();
	attackKeyframeObject_->Reset();
	isPlayedAttackKeyframe_ = false;

	// 補間座標を決定
	hideStartPos_ = bossEnemy_->GetTranslation();
	hideTargetPos_ = hideStartPos_ + bossEnemy_->GetTransform().GetForward() * hideDistance_;

	// 親を設定
	attackKeyframeObject_->SetParent(player_->GetTag().name, player_->GetTransform());
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
		}
	} else {

		// 時間の更新
		hideEnemyTimer_.Update();

		// 座標の補間とα値の補間
		Vector3 lerpPos = Vector3::Lerp(hideStartPos_, hideTargetPos_, hideEnemyTimer_.easedT_);
		bossEnemy_->SetTranslation(lerpPos);

		// トリガーで雷を発生させる
		if (hideEnemyTimer_.IsReached()) {

			// 雷攻撃発生
			EmitLightningAttack();

			// 一度画面外に飛ばす
			bossEnemy_->SetTranslation(Vector3(0.0f, -32, 0.0f));
		}
	}
}

void BossEnemyGreatAttackInOutArea::UpdateIn() {

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

	// キーフレーム補間終了後状態終了
	if (!attackKeyframeObject_->IsUpdating()) {

		canExit_ = true;
	}
}

void BossEnemyGreatAttackInOutArea::EmitLightningAttack() {

	// n番目ごとに進める角度
	float stepAngle = pi * 2.0f / static_cast<float>(lightningCount_);

	// 雷攻撃発生
	for (uint32_t i = 0; i <= lightningCount_; ++i) {

		// 現在の角度
		float angle = stepAngle * static_cast<float>(i);
		float sin = std::sin(angle);
		float cos = std::cos(angle);

		// 発生位置
		Vector3 emitPos = Vector3(sin * outAreaRadius_, 0.0f, cos * outAreaRadius_);

		// エフェクト発生
		lightningAttackEffects_[i]->Emit(emitPos);
	}
}

void BossEnemyGreatAttackInOutArea::UpdateAlways() {

	// エフェクト更新
	for (const auto& effect : lightningAttackEffects_) {

		effect->Update();
	}
	// キーフレーム更新
	attackKeyframeObject_->UpdateKey();
}

void BossEnemyGreatAttackInOutArea::Exit() {

	// リセット
	canExit_ = false;
	currentState_ = State::Out;
	isPlayedAttackKeyframe_ = false;
}

void BossEnemyGreatAttackInOutArea::ImGui() {

	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	ImGui::SeparatorText("Out");

	if (ImGui::Button("Emit Lightning")) {

		EmitLightningAttack();
	}

	ImGui::DragFloat("OutAreaRadius", &outAreaRadius_, 0.1f);
	int32_t lightningCount = static_cast<int32_t>(lightningCount_);
	ImGui::DragInt("LightningCount", &lightningCount, 1, 1);
	lightningCount_ = std::clamp(static_cast<uint32_t>(lightningCount), uint32_t(0), maxLightningCount_);

	ImGui::DragFloat("HideDistance", &hideDistance_, 0.1f);

	hideEnemyTimer_.ImGui("HideEnemyTimer");
	lightningAttackTimer_.ImGui("LightningAttackTimer");

	// 円の描画
	lineRenderer->DrawCircle(lightningCount, outAreaRadius_,
		Vector3(0.0f, 1.0f, 0.0f), Color::Cyan());

	ImGui::SeparatorText("In");

	int32_t attackKeyframeIndex = static_cast<int32_t>(attackKeyframeIndex_);
	ImGui::DragInt("AttackKeyframeIndex", &attackKeyframeIndex, 1, 0);
	attackKeyframeIndex_ = static_cast<uint32_t>(attackKeyframeIndex);

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
	lightningCount_ = std::clamp(lightningCount_, uint32_t(0), maxLightningCount_);
	hideDistance_ = data.value("hideDistance_", 5.0f);
	hideEnemyTimer_.FromJson(data.value("hideEnemyTimer_", Json()));
	lightningAttackTimer_.FromJson(data.value("lightningAttackTimer_", Json()));
	attackKeyframeObject_->FromJson(data.value("AttackKeyframeObject", Json()));
}

void BossEnemyGreatAttackInOutArea::SaveJson(Json& data) {

	data["outAreaRadius_"] = outAreaRadius_;
	data["lightningCount_"] = lightningCount_;
	data["hideDistance_"] = hideDistance_;
	data["attackKeyframeIndex_"] = attackKeyframeIndex_;
	hideEnemyTimer_.ToJson(data["hideEnemyTimer_"]);
	lightningAttackTimer_.ToJson(data["lightningAttackTimer_"]);
	attackKeyframeObject_->ToJson(data["AttackKeyframeObject"]);
}
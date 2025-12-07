#include "BossEnemyGreatAttackInOutArea.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>

//============================================================================
//	BossEnemyGreatAttackInOutArea classMethods
//============================================================================

BossEnemyGreatAttackInOutArea::BossEnemyGreatAttackInOutArea() {

	// 雷攻撃
	for (auto& effect : lightningAttackEffects_) {

		effect = std::make_unique<EffectGroup>();
		effect->Init("lightningAttack", "BossEnemyEffect");
		effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightningAttackEffect.json");
	}
}

void BossEnemyGreatAttackInOutArea::Enter() {

	canExit_ = true;

	// 状態初期化
	currentState_ = State::Out;
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


}

void BossEnemyGreatAttackInOutArea::UpdateIn() {


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
}

void BossEnemyGreatAttackInOutArea::Exit() {


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

	// 円の描画
	lineRenderer->DrawCircle(lightningCount, outAreaRadius_,
		Vector3(0.0f, 1.0f, 0.0f), Color::Cyan());

	ImGui::SeparatorText("In");

}

void BossEnemyGreatAttackInOutArea::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	outAreaRadius_ = data.value("outAreaRadius_", 10.0f);
	lightningCount_ = data.value("lightningCount_", 8u);
	lightningCount_ = std::clamp(lightningCount_, uint32_t(0), maxLightningCount_);
}

void BossEnemyGreatAttackInOutArea::SaveJson(Json& data) {

	data["outAreaRadius_"] = outAreaRadius_;
	data["lightningCount_"] = lightningCount_;
}
#include "BossEnemyRushAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyRushAttackState classMethods
//============================================================================

void BossEnemyRushAttackState::InitBlade() {

	// 3本の刃
	for (uint32_t index = 0; index < bladeMaxCount_; ++index) {

		divisionBlades_[index] = std::make_unique<BossEnemyBladeCollision>();
		divisionBlades_[index]->Init("divisionBlade");
	}

	// 1本の刃
	singleBlade_ = std::make_unique<BossEnemyBladeCollision>();
	singleBlade_->Init("singleBlade_Rush");
}

BossEnemyRushAttackState::BossEnemyRushAttackState() {

	// 最大攻撃回数を設定
	maxAttackCount_ = static_cast<int>(pattern_.size());

	// 刃を初期化
	InitBlade();
}

void BossEnemyRushAttackState::Enter() {

	// 最初の設定
	currentState_ = State::Teleport;
	// テレポート状態でanimationを設定
	bossEnemy_->SetNextAnimation("bossEnemy_teleport", false, nextAnimDuration_);
	canExit_ = false;

	// 座標設定
	SakuEngine::Vector3 center = player_->GetTranslation();
	center.y = 0.0f;
	const SakuEngine::Vector3 forward = followCamera_->GetTransform().GetForward();
	startPos_ = bossEnemy_->GetTranslation();
	targetPos_ = SakuEngine::Math::RandomPointOnArcInSquare(center, forward,
		farRadius_, halfAngle_, SakuEngine::Vector3::AnyInit(0.0f), moveClampSize_ / 2.0f);

	currentAlpha_ = 1.0f;
	bossEnemy_->SetAlpha(currentAlpha_);
	bossEnemy_->SetCastShadow(true);

	// playerの方を向かせる
	SakuEngine::Math::RotateToDirection3D(*bossEnemy_, SakuEngine::Math::GetDirection3D(*bossEnemy_, *player_), rotationLerpRate_);
}

void BossEnemyRushAttackState::UpdateAlways() {

	// 衝突更新
	for (const auto& divisionBlade : divisionBlades_) {

		divisionBlade->Update();
	}
	singleBlade_->Update();
}

void BossEnemyRushAttackState::Update() {

	const float deltaTime = SakuEngine::GameTimer::GetScaledDeltaTime();
	switch (currentState_) {
	case State::Teleport: {

		// テレポートの更新
		UpdateTeleport(deltaTime);
		break;
	}
	case State::Attack: {

		// 攻撃更新
		UpdateAttack();
		break;
	}
	case State::Cooldown: {

		// クールタイム更新
		UpdateCooldown(deltaTime);
		break;
	}
	}

	// 衝突、刃の更新処理
	UpdateBlade();

	// 攻撃回数が最大を超えたら遷移可能状態にする
	if (maxAttackCount_ <= currentAttackCount_) {

		canExit_ = true;
	}
}

void BossEnemyRushAttackState::UpdateTeleport(float deltaTime) {

	lerpTimer_ += deltaTime;
	float lerpT = std::clamp(lerpTimer_ / lerpTime_, 0.0f, 1.0f);
	lerpT = EasedValue(easingType_, lerpT);

	// 座標補完
	bossEnemy_->SetTranslation(SakuEngine::Vector3::Lerp(startPos_, targetPos_, lerpT));
	SakuEngine::Math::RotateToDirection3D(*bossEnemy_, SakuEngine::Math::GetDirection3D(*bossEnemy_, *player_), rotationLerpRate_);

	const float disappearEnd = fadeOutTime_;           // 消え終わる時間
	const float appearStart = lerpTime_ - fadeInTime_; // 現れ始める時間

	bossEnemy_->SetCastShadow(true);
	if (lerpTimer_ <= disappearEnd) {

		float t = std::clamp(lerpTimer_ / fadeOutTime_, 0.0f, 1.0f);
		currentAlpha_ = 1.0f - t;
	} else if (lerpTimer_ >= appearStart) {

		float t = std::clamp((lerpTimer_ - appearStart) / fadeInTime_, 0.0f, 1.0f);
		currentAlpha_ = t;
	} else {

		currentAlpha_ = 0.0f;
		bossEnemy_->SetCastShadow(false);
	}
	bossEnemy_->SetAlpha(currentAlpha_);

	// tが1.0fになったら攻撃animationに切り替える
	if (1.0f <= lerpT) {
		if (currentAttackCount_ < pattern_.size()) {

			// 攻撃アニメーションへ切り替え
			bossEnemy_->SetNextAnimation(pattern_[currentAttackCount_].animationName, false, nextAnimDuration_);
			bossEnemy_->SetTranslation(targetPos_);

			bossEnemy_->SetAlpha(1.0f);
			bossEnemy_->SetCastShadow(true);

			currentState_ = State::Attack;
			lerpTimer_ = 0.0f;
		}
	}
}

void BossEnemyRushAttackState::UpdateAttack() {

	// 攻撃animationが終了したら攻撃クールダウン状態に遷移させる
	if (bossEnemy_->IsAnimationFinished()) {

		currentState_ = State::Cooldown;
		attackCoolTimer_ = 0.0f;
	}
}

void BossEnemyRushAttackState::UpdateCooldown(float deltaTime) {

	attackCoolTimer_ += deltaTime;
	if (attackCoolTime_ <= attackCoolTimer_) {

		// 次の攻撃に進める
		++currentAttackCount_;

		// テレポート設定
		currentState_ = State::Teleport;
		lerpTimer_ = 0.0f;

		// テレポート状態でanimationを設定
		bossEnemy_->SetNextAnimation("bossEnemy_teleport", false, nextAnimDuration_);

		// 座標設定
		SakuEngine::Vector3 center = player_->GetTranslation();
		center.y = 0.0f;
		const SakuEngine::Vector3 forward = followCamera_->GetTransform().GetForward();
		startPos_ = bossEnemy_->GetTranslation();
		targetPos_ = SakuEngine::Math::RandomPointOnArcInSquare(center, forward,
			farRadius_, halfAngle_, SakuEngine::Vector3::AnyInit(0.0f), moveClampSize_ / 2.0f);

		// playerの方を向かせる
		SakuEngine::Math::RotateToDirection3D(*bossEnemy_, SakuEngine::Math::GetDirection3D(*bossEnemy_, *player_), rotationLerpRate_);
	}
}

void BossEnemyRushAttackState::UpdateBlade() {

	if (currentAttackCount_ == maxAttackCount_) {
		return;
	}

	// 最後の攻撃か判定
	bool isLastAttack = (currentAttackCount_ == maxAttackCount_ - 1);

	if (isLastAttack) {
		if (bossEnemy_->IsEventKey("Attack", 0)) {

			EmitSingleBlade();
		}
	} else {
		if (bossEnemy_->IsEventKey("Attack", 0)) {

			EmitDivisionBlades();
		}
	}
}

SakuEngine::Vector3 BossEnemyRushAttackState::CalcBaseDir() const {

	return (player_->GetTranslation() - bossEnemy_->GetTranslation()).Normalize();
}

SakuEngine::Vector3 BossEnemyRushAttackState::CalcDivisionBladeDir(uint32_t index) const {

	const float offset[bladeMaxCount_] = { -divisionOffsetAngle_, 0.0f, divisionOffsetAngle_ };
	return SakuEngine::Math::RotateY(CalcBaseDir(), offset[index] * SakuEngine::pi / 180.0f);
}

void BossEnemyRushAttackState::EmitDivisionBlades() {

	// 発生処理
	const SakuEngine::Vector3 pos = bossEnemy_->GetTranslation();
	for (uint32_t i = 0; i < bladeMaxCount_; ++i) {

		const SakuEngine::Vector3 velocity = CalcDivisionBladeDir(i) * divisionBladeMoveSpeed_;
		divisionBlades_[i]->EmitEffect(pos, velocity);
	}
}

void BossEnemyRushAttackState::EmitSingleBlade() {

	// 発生処理
	const SakuEngine::Vector3 pos = bossEnemy_->GetTranslation();
	const SakuEngine::Vector3 velocity = CalcBaseDir() * singleBladeMoveSpeed_;
	singleBlade_->EmitEffect(pos, velocity);
}

void BossEnemyRushAttackState::Exit() {

	// リセット
	canExit_ = false;
	lerpTimer_ = 0.0f;
	attackCoolTimer_ = 0.0f;
	currentAttackCount_ = 0;
	currentAlpha_ = 1.0f;
	bossEnemy_->SetAlpha(currentAlpha_);
	bossEnemy_->SetCastShadow(true);
}

void BossEnemyRushAttackState::ImGui() {

	if (ImGui::CollapsingHeader("RushAttackState")) {

		ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
		ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);

		ImGui::Text(std::format("canExit: {}", canExit_).c_str());
		ImGui::Text("currentAttack: %d / %d", currentAttackCount_, maxAttackCount_);
		ImGui::Text("attackCoolTime: %.3f / %.3f", attackCoolTimer_, attackCoolTime_);
		ImGui::DragFloat("farRadius:Red", &farRadius_, 0.1f);
		ImGui::DragFloat("nearRadius:Blue", &nearRadius_, 0.1f);
		ImGui::DragFloat("halfAngle", &halfAngle_, 0.1f);
		ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
		ImGui::DragFloat("fadeOutTime", &fadeOutTime_, 0.01f);
		ImGui::DragFloat("fadeInTime", &fadeInTime_, 0.01f);
		ImGui::DragFloat("attackCoolTime", &attackCoolTime_, 0.01f);
		ImGui::DragFloat("divisionOffsetAngle_", &divisionOffsetAngle_, 0.01f);
		ImGui::DragFloat("divisionBladeMoveSpeed", &divisionBladeMoveSpeed_, 0.1f);
		ImGui::DragFloat("singleBladeMoveSpeed", &singleBladeMoveSpeed_, 0.1f);
		Easing::SelectEasingType(easingType_);

		{
			const SakuEngine::Vector3 bossPos = bossEnemy_->GetTranslation();
			const SakuEngine::Vector3 toPlayer = (player_->GetTranslation() - bossPos).Normalize();
			const float   baseYaw = divisionOffsetAngle_ * (SakuEngine::pi / 180.0f);
			const float   angles[bladeMaxCount_] = { -baseYaw, 0.0f, baseYaw };

			for (uint32_t i = 0; i < bladeMaxCount_; ++i) {
				SakuEngine::Vector3 dir = SakuEngine::Math::RotateY(toPlayer, angles[i]);
				SakuEngine::Vector3 lineStart = bossPos + SakuEngine::Vector3(0.0f, 4.0f, 0.0f);
				SakuEngine::Vector3 lineEnd = lineStart + dir * 128.0f;

				SakuEngine::LineRenderer::GetInstance()->DrawLine3D(lineStart, lineEnd, SakuEngine::Color::Red());
			}
		}

		{
			SakuEngine::Vector3 center = player_->GetTranslation();
			center.y = 4.0f;
			SakuEngine::LineRenderer::GetInstance()->DrawArc(8, farRadius_, halfAngle_,
				center, followCamera_->GetTransform().GetForward(), SakuEngine::Color::Red());
			SakuEngine::LineRenderer::GetInstance()->DrawArc(8, nearRadius_, halfAngle_,
				center, followCamera_->GetTransform().GetForward(), SakuEngine::Color::Blue());
		}
	}

	if (ImGui::CollapsingHeader("Blade")) {

		// 発生させる
		if (ImGui::Button("Emit DivisionBlade")) {

			EmitDivisionBlades();
		}
		if (ImGui::Button("Emit SingleBlade")) {

			EmitSingleBlade();
		}

		ImGui::Separator();

		// 真ん中
		divisionBlades_[1]->ImGui();
		singleBlade_->ImGui();

		// 衝突更新
		for (const auto& divisionBlade : divisionBlades_) {

			divisionBlade->Update();
		}
		singleBlade_->Update();
	}

	if (ImGui::CollapsingHeader("Blade Effect")) {

		ImGui::DragFloat("singleBladeScaling", &singleBladeEffectScalingValue_, 0.01f);
	}
}

void BossEnemyRushAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = SakuEngine::JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	farRadius_ = SakuEngine::JsonAdapter::GetValue<float>(data, "farRadius_");
	nearRadius_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nearRadius_");
	halfAngle_ = SakuEngine::JsonAdapter::GetValue<float>(data, "halfAngle_");
	lerpTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "lerpTime_");
	attackCoolTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "attackCoolTime_");
	fadeOutTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "fadeOutTime_");
	fadeInTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "fadeInTime_");
	divisionOffsetAngle_ = SakuEngine::JsonAdapter::GetValue<float>(data, "divisionOffsetAngle_");
	divisionBladeMoveSpeed_ = SakuEngine::JsonAdapter::GetValue<float>(data, "divisionBladeMoveSpeed_");
	singleBladeMoveSpeed_ = SakuEngine::JsonAdapter::GetValue<float>(data, "singleBladeMoveSpeed_");
	easingType_ = static_cast<EasingType>(SakuEngine::JsonAdapter::GetValue<int>(data, "easingType_"));
	singleBladeEffectScalingValue_ = data.value("singleBladeEffectScalingValue_", 1.0f);

	{
		Json clampData;
		if (SakuEngine::JsonAdapter::LoadCheck("GameConfig/gameConfig.json", clampData)) {

			moveClampSize_ = SakuEngine::JsonAdapter::GetValue<float>(clampData["playableArea"], "length");
		}
	}
}

void BossEnemyRushAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["farRadius_"] = farRadius_;
	data["nearRadius_"] = nearRadius_;
	data["halfAngle_"] = halfAngle_;
	data["lerpTime_"] = lerpTime_;
	data["attackCoolTime_"] = attackCoolTime_;
	data["fadeOutTime_"] = fadeOutTime_;
	data["fadeInTime_"] = fadeInTime_;
	data["divisionOffsetAngle_"] = divisionOffsetAngle_;
	data["divisionBladeMoveSpeed_"] = divisionBladeMoveSpeed_;
	data["singleBladeMoveSpeed_"] = singleBladeMoveSpeed_;
	data["singleBladeEffectScalingValue_"] = singleBladeEffectScalingValue_;
	data["easingType_"] = static_cast<int>(easingType_);
}
#include "BossEnemyAllFieldAttackState.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	BossEnemyAllFieldAttackState classMethods
//============================================================================

BossEnemyAllFieldAttackState::BossEnemyAllFieldAttackState() {

	// 突起攻撃の突起初期化
	for (auto& protrusion : protrusions_) {

		protrusion = std::make_unique<BossEnemyProtrusion>();
		protrusion->Init();
	}

	// 範囲攻撃のエフェクト初期化
	areaAttackEffect_ = std::make_unique<EffectGroup>();
	areaAttackEffect_->Init("fieldAreaAttack", "BossEnemyEffect");
	areaAttackEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyAreaAttackEffect.json");
}

void BossEnemyAllFieldAttackState::Enter() {

	// 状態の初期化
	currentState_ = State::Previous;
	areaAttackState_ = AreaAttackState::Charge;
	canExit_ = false;

	// アニメーション座標の設定
	moveToCenterAnim_.SetStart(bossEnemy_->GetTransform().translation);
	moveToCenterAnim_.SetEnd(Vector3::AnyInit(0.0f));
	// アニメーション開始
	moveToCenterAnim_.Start();
	// アイドル状態にする
	bossEnemy_->SetNextAnimation("bossEnemy_idle", false, 0.2f);
}

void BossEnemyAllFieldAttackState::Update() {

	// 状態毎の更新
	switch (currentState_) {
	case BossEnemyAllFieldAttackState::State::Previous:

		PrepareProtrusionAttack();
		break;
	case BossEnemyAllFieldAttackState::State::Protrusion:

		UpdateProtrusionEmit();
		break;
	case BossEnemyAllFieldAttackState::State::Area:

		UpdateAreaAttack();
		break;
	case BossEnemyAllFieldAttackState::State::Finish:

		// 今はここで終了
		canExit_ = true;
		break;
	}
}

void BossEnemyAllFieldAttackState::PrepareProtrusionAttack() {

	// アニメーション更新
	// 座標
	Vector3 currentPos = bossEnemy_->GetTransform().translation;
	moveToCenterAnim_.LerpValue(currentPos);
	bossEnemy_->SetTranslation(currentPos);
	// α値
	float progress = moveToCenterAnim_.GetProgress();
	// 0.0~0.5で透明(0.0f)に補間し、0.5~1.0で補間して元に戻す(1.0f)
	float alpha = 0.0f;
	if (progress < 0.5f) {

		float t = progress / 0.5f;
		float eased = EaseOutExpo(t);
		alpha = 1.0f - eased;
	} else {

		float t = (progress - 0.5f) / 0.5f;
		float eased = EaseInExpo(t);
		alpha = eased;
	}
	bossEnemy_->SetAlpha(alpha);

	// アニメーション終了後、次の状態へ
	if (moveToCenterAnim_.IsFinished()) {

		moveToCenterAnim_.Reset();
		currentState_ = State::Protrusion;
		playerDistanceToCenter_ = protrusionEmitRadiusMin_;

		// 地面に叩きつけるアニメーションにする
		bossEnemy_->SetNextAnimation("bossEnemy_groundSmash", false, 0.2f);
	}
}

void BossEnemyAllFieldAttackState::UpdateProtrusionEmit() {

	// アニメーション再生終了後、突起を発生させる
	if (bossEnemy_->IsAnimationFinished()) {

		// 全て発生
		EmitAll();
		// 次の状態へ
		currentState_ = State::Area;
	}
}

void BossEnemyAllFieldAttackState::EmitAll() {

	// n番目ごとに進める角度
	float stepAngle = pi * 2.0f / static_cast<float>(maxProtrusionCount_);
	float offset = stepAngle * 0.5f;

	// 個数で分割して発生させる角度を決める
	for (uint32_t i = 0; i < maxProtrusionCount_; ++i) {

		// 現在の角度
		float angle = stepAngle * static_cast<float>(i) + offset;
		float sin = std::sin(angle);
		float cos = std::cos(angle);

		// 発生位置
		Vector3 emitPos = Vector3(sin * protrusionEmitRadiusMin_, 0.0f, cos * protrusionEmitRadiusMin_);
		// 発生方向
		Vector3 direction = Vector3(cos, 0.0f, -sin).Normalize();

		// 発生
		protrusions_[i]->Emit(emitPos, direction, protrusionLifeTime_);
	}
}

void BossEnemyAllFieldAttackState::UpdateAreaAttack() {

	switch (areaAttackState_) {
	case BossEnemyAllFieldAttackState::AreaAttackState::Charge:

		// チャージ時間を更新
		areaAttackChargeTimer_.Update();

		// チャージ時間終了で次の状態へ
		if (areaAttackChargeTimer_.IsReached()) {

			// タイマーリセット
			areaAttackChargeTimer_.Reset();
			// 次の状態へ
			areaAttackState_ = AreaAttackState::Emit;

			// 発生可能にする
			areaAttackEffect_->SetLifeEndMode("", ParticleLifeEndMode::Advance, true);
		}
		break;
	case BossEnemyAllFieldAttackState::AreaAttackState::Emit:

		// エフェクト発生
		areaAttackEffect_->Emit(Vector3::AnyInit(0.0f));

		// そのまま次の状態へ
		areaAttackState_ = AreaAttackState::Wait;
		break;
	case BossEnemyAllFieldAttackState::AreaAttackState::Wait:

		// 範囲攻撃時間を更新
		areaAttackTimer_.Update();

		// 一定間隔おきにコライダーを発生させる
		areaAttackEmitIntervalTimer_.Update();
		if (areaAttackEmitIntervalTimer_.IsReached()) {

			// タイマーリセット
			areaAttackEmitIntervalTimer_.Reset();
			// コライダー発生
			EmitAreaAttackCollider();
		}

		// 範囲攻撃時間終了で終了
		if (areaAttackTimer_.IsReached()) {

			// タイマーリセット
			areaAttackTimer_.Reset();
			currentState_ = State::Finish;
			areaAttackEffect_->SetLifeEndMode("", ParticleLifeEndMode::Kill, true);

			// コライダーをすべて消す
			areaAttackColliders_.clear();
		}
		break;
	}
}

void BossEnemyAllFieldAttackState::CreateAreaAttackCollider(
	bool isLong, const SakuEngine::Vector3& direction) {

	// コライダーの生成
	AreaAttackCollider collider{};
	collider.collider = std::make_unique<Collider>();
	CollisionBody* collisionBody = collider.collider->AddCollider(CollisionShape::Sphere(), true);
	// 半径を設定
	collider.collider->SetSphereRadius(areaAttackColliderRadius_);
	// タイプ設定、最初は当たらないようにする
	collisionBody->SetType(ColliderType::Type_BossWeapon);
	collisionBody->SetTargetType(ColliderType::Type_Player);
	collider.transform.Init();
	// ライフタイムを設定
	collider.lifeTimer.target_ = isLong ? longColliderLifeTime_ : shortColliderLifeTime_;
	// 移動速度を設定
	collider.velocity = direction * areaAttackColliderSpeed_;

	// 追加
	areaAttackColliders_.emplace_back(std::move(collider));
}

void BossEnemyAllFieldAttackState::EmitAreaAttackCollider() {

	// n番目ごとに進める角度
	// 0.2radずつ進める
	float stepAngle = emitAngleInterval_;

	// 0 ～ 2π 未満まで繰り返す
	for (float angle = 0.0f; angle < pi * 2.0f; angle += stepAngle) {

		float sinValue = std::sin(angle);
		float cosValue = std::cos(angle);

		// 発生方向
		Vector3 direction = Vector3(cosValue, 0.0f, -sinValue);
		// 壁に当たるかどうかの判定
		// TODO

		// コライダー発生
		CreateAreaAttackCollider(true, direction);
	}
}

void BossEnemyAllFieldAttackState::UpdateAlways() {

	// 突起更新
	for (const auto& protrusion : protrusions_) {

		protrusion->Update();
	}
	// 範囲攻撃エフェクト更新
	areaAttackEffect_->Update();

	// コライダー更新
	for (auto& areaCollider : areaAttackColliders_) {

		// 移動
		areaCollider.transform.translation += areaCollider.velocity;
		// ライフタイム更新
		areaCollider.lifeTimer.Update();

		// コライダーの位置を更新
		areaCollider.transform.UpdateMatrix();
		areaCollider.collider->UpdateAllBodies(areaCollider.transform);
	}
}

void BossEnemyAllFieldAttackState::Exit() {

	// リセット
	canExit_ = false;
	moveToCenterAnim_.Reset();
}

void BossEnemyAllFieldAttackState::ImGui() {

	ImGui::Text("state:%s", EnumAdapter<State>::ToString(currentState_));

	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Previous")) {

		moveToCenterAnim_.ImGui("MoveToCenterAnim", false);
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Protrusion")) {

		if (ImGui::Button("Emit All")) {

			EmitAll();
		}

		ImGui::DragFloat("protrusionEmitRadiusMin", &protrusionEmitRadiusMin_, 0.01f);
		ImGui::DragFloat("protrusionLifeTime", &protrusionLifeTime_, 0.01f);

		ImGui::Spacing();

		protrusions_.front()->ImGui();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Area Attack")) {

		ImGui::Text("areaAttackState: %s", EnumAdapter<AreaAttackState>::ToString(areaAttackState_));
		areaAttackChargeTimer_.ImGui("AreaAttackChargeTimer");
		areaAttackTimer_.ImGui("AreaAttackTimer");

		ImGui::Spacing();
		ImGui::SeparatorText("Collision");

		ImGui::Text("Colliders Count: %d", static_cast<int>(areaAttackColliders_.size()));

		ImGui::DragFloat("areaAttackColliderRadius", &areaAttackColliderRadius_, 0.01f);
		ImGui::DragFloat("longColliderLifeTime", &longColliderLifeTime_, 0.01f);
		ImGui::DragFloat("shortColliderLifeTime", &shortColliderLifeTime_, 0.01f);
		ImGui::DragFloat("emitAngleInterval", &emitAngleInterval_, 0.01f);
		ImGui::DragFloat("areaAttackColliderSpeed", &areaAttackColliderSpeed_, 0.01f);
		areaAttackEmitIntervalTimer_.ImGui("AreaAttackEmitIntervalTimer");
	}
}

void BossEnemyAllFieldAttackState::ApplyJson(const Json& data) {

	protrusionEmitRadiusMin_ = data.value("protrusionEmitRadiusMin_", 0.0f);
	protrusionLifeTime_ = data.value("protrusionLifeTime_", 0.0f);

	moveToCenterAnim_.FromJson(data.value("moveToCenterAnim_", Json()));

	areaAttackChargeTimer_.FromJson(data.value("areaAttackChargeTimer_", Json()));
	areaAttackTimer_.FromJson(data.value("areaAttackTimer_", Json()));

	areaAttackColliderRadius_ = data.value("areaAttackColliderRadius_", 0.0f);
	longColliderLifeTime_ = data.value("longColliderLifeTime_", 0.0f);
	shortColliderLifeTime_ = data.value("shortColliderLifeTime_", 0.0f);
	emitAngleInterval_ = data.value("emitAngleInterval_", 0.0f);
	areaAttackColliderSpeed_ = data.value("areaAttackColliderSpeed_", 0.0f);

	areaAttackEmitIntervalTimer_.FromJson(data.value("areaAttackEmitIntervalTimer_", Json()));
}

void BossEnemyAllFieldAttackState::SaveJson(Json& data) {

	data["protrusionEmitRadiusMin_"] = protrusionEmitRadiusMin_;
	data["protrusionLifeTime_"] = protrusionLifeTime_;

	moveToCenterAnim_.ToJson(data["moveToCenterAnim_"]);
	areaAttackChargeTimer_.ToJson(data["areaAttackChargeTimer_"]);
	areaAttackTimer_.ToJson(data["areaAttackTimer_"]);

	data["areaAttackColliderRadius_"] = areaAttackColliderRadius_;
	data["longColliderLifeTime_"] = longColliderLifeTime_;
	data["shortColliderLifeTime_"] = shortColliderLifeTime_;
	data["emitAngleInterval_"] = emitAngleInterval_;
	data["areaAttackColliderSpeed_"] = areaAttackColliderSpeed_;

	areaAttackEmitIntervalTimer_.ToJson(data["areaAttackEmitIntervalTimer_"]);
}
#include "BossEnemyProtrusion.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	BossEnemyProtrusion classMethods
//============================================================================

void BossEnemyProtrusion::CreateEffect() {

	std::unique_ptr<EffectGroup> protrusion = std::make_unique<EffectGroup>();
	protrusion->Init("protrusionAttack", "BossEnemyEffect");
	protrusion->LoadJson("GameEffectGroup/BossEnemy/bossEnemyProtrusionAttackEffect.json");
	protrusions_.emplace_back(std::move(protrusion));
}

void BossEnemyProtrusion::Init() {

	collisionTransform_.Init();
	collisionTransform_.translation = collisionSafePos_;
	// 衝突初期化
	collider_ = std::make_unique<Collider>();
	// OBBで追加
	collisionBody_ = collider_->AddCollider(CollisionShape::OBB(), true);
	collider_->InitBodyOffsets(collisionBody_->GetShape());
	// タイプ設定、最初は当たらないようにする
	collisionBody_->SetType(ColliderType::Type_None);
	collisionBody_->SetTargetType(ColliderType::Type_Player);
	collisionTransform_.Init();
	collisionTransform_.translation = collisionSafePos_;

	// jsonの適用
	ApplyJson();

	// 初期化
	demoProtrusion_ = std::make_unique<EffectGroup>();
	demoProtrusion_->Init("protrusionAttack", "BossEnemyEffect");
	demoProtrusion_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyProtrusionAttackEffect.json");

	// すべて削除する
	protrusions_.clear();
	// 個数分の突起エフェクトを作成
	for (uint32_t i = 0; i < maxProtrusionCount_; ++i) {

		// エフェクト作成
		CreateEffect();
	}
}

void BossEnemyProtrusion::Emit(const Vector3& pos,
	const Vector3& direction, float lifeTime) {

	emitInfo_.pos = pos;
	emitInfo_.direction = direction;
	// 全体のライフタイムを設定
	lifeTimer_.target_ = lifeTime;

	// 状態を発生中にする
	currentState_ = State::Emit;
	emittedProtrusionCount_ = 0;

	// エフェクトをプログラム管理のライフタイムにする
	for (const auto& protrusion : protrusions_) {

		protrusion->SetLifeEndMode("bossProtrusion", ParticleLifeEndMode::Advance, true);
	}

	// 衝突判定を有効にする
	collisionBody_->SetType(ColliderType::Type_BossWeapon);
	// 回転を設定
	collisionTransform_.rotation = Quaternion::LookRotation(direction, Vector3(1.0f, 0.0f, 0.0f));
	collisionTransform_.rotation = Quaternion::Normalize(collisionTransform_.rotation);
	// 座標を設定、スケール分計算して中心にする
	collisionTransform_.translation = pos + direction * collisionTransform_.scale.z;
}

void BossEnemyProtrusion::Update() {

	switch (currentState_) {
	case BossEnemyProtrusion::State::Emit: {

		// 最大個数に達していなければ発生させる
		bool enableEmit = emittedProtrusionCount_ < maxProtrusionCount_;
		if (enableEmit) {

			// 発生時間を更新
			emitTimer_.Update(std::nullopt, false);

			// 時間経過で発生
			if (emitTimer_.IsReached()) {

				// 発生位置を設定
				Vector3 emitPos = emitInfo_.pos + emitInfo_.direction *
					emitPosOffset_ * static_cast<float>(emittedProtrusionCount_);
				emitPos.y = RandomGenerator::Generate(-emitPosYRandomRange_, emitPosYRandomRange_);

				// 指定インデックスのエフェクトを発生させる
				protrusions_[emittedProtrusionCount_]->Emit(emitPos);

				// 発生回数+1
				++emittedProtrusionCount_;
				emitTimer_.Reset();
			}
		} else {

			// 発生時間を更新
			emitTimer_.Update(0.4f, false);

			if (emitTimer_.IsReached()) {

				// 増やす
				++emittedProtrusionCount_;
				emitTimer_.Reset();
				// 1回分余分に時間を更新させる
				if (emittedProtrusionCount_ < maxProtrusionCount_ + 2) {
					return;
				}

				// 消さないで待たせる
				for (const auto& protrusion : protrusions_) {

					protrusion->SetLifeEndMode("bossProtrusion", ParticleLifeEndMode::Clamp, true);
				}
				currentState_ = State::Wait;
			}
		}
		break;
	}
	case BossEnemyProtrusion::State::Wait:

		// ライフタイムを更新
		lifeTimer_.Update(std::nullopt, false);

		// ライフタイム終了で状態をRemoveにする
		if (lifeTimer_.IsReached()) {

			lifeTimer_.Reset();
			currentState_ = State::Remove;
		}
		break;
	case BossEnemyProtrusion::State::Remove:

		// すべてのエフェクトを削除
		for (const auto& protrusion : protrusions_) {

			protrusion->SetLifeEndMode("bossProtrusion", ParticleLifeEndMode::Kill, true);
		}

		// 衝突判定を無効にする
		collisionBody_->SetType(ColliderType::Type_None);
		collisionTransform_.translation = collisionSafePos_;

		// 状態を戻す
		currentState_ = State::None;
		break;
	}

	// 更新
	demoProtrusion_->Update();
	for (const auto& protrusion : protrusions_) {

		protrusion->Update();
	}
	// 衝突判定更新
	collisionTransform_.UpdateMatrix();
	collider_->UpdateAllBodies(collisionTransform_);
}

void BossEnemyProtrusion::ImGui() {

	if (ImGui::Button("Save Json##BossEnemyProtrusion")) {

		SaveJson();
	}
	ImGui::Separator();

	if (ImGui::Button("Emit")) {

		Emit(Vector3::AnyInit(0.0f), Vector3(1.0f, 0.0f, 0.0f), 1.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("CreateEffect")) {

		// すべて削除する
		protrusions_.clear();
		// 個数分の突起エフェクトを作成
		for (uint32_t i = 0; i < maxProtrusionCount_; ++i) {

			CreateEffect();
		}
	}

	if (ImGui::CollapsingHeader("Parameters")) {

		ImGui::DragFloat("emitPosOffset", &emitPosOffset_, 0.1f);
		ImGui::DragFloat("emitPosYRandomRange", &emitPosYRandomRange_, 0.01f);
		ImGuiHelper::DragUint32("maxProtrusionCount", maxProtrusionCount_, 100);
		ImGui::DragFloat3("collisionScale", &collisionTransform_.scale.x, 0.01f);
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Timer")) {

		emitTimer_.ImGui("emitTimer");
		lifeTimer_.ImGui("lifeTimer");
	}
}

void BossEnemyProtrusion::ApplyJson() {

	Json data{};
	if (!JsonAdapter::LoadCheck("BossEnemy/Item/protrusion.json", data)) {
		return;
	}

	maxProtrusionCount_ = data.value("maxProtrusionCount_", maxProtrusionCount_);
	emitPosYRandomRange_ = data.value("emitPosYRandomRange", emitPosYRandomRange_);
	emitPosOffset_ = data.value("emitPosOffset", emitPosOffset_);

	emitTimer_.FromJson(data.value("emitTimer_", Json()));
	lifeTimer_.FromJson(data["lifeTimer_"]);

	collisionTransform_.scale = Vector3::FromJson(data.value("collisionTransform_.scale", Json()));
}

void BossEnemyProtrusion::SaveJson() {

	Json data{};

	data["maxProtrusionCount_"] = maxProtrusionCount_;
	data["emitPosYRandomRange"] = emitPosYRandomRange_;
	data["emitPosOffset"] = emitPosOffset_;

	emitTimer_.ToJson(data["emitTimer_"]);
	lifeTimer_.ToJson(data["lifeTimer_"]);

	data["collisionTransform_.scale"] = collisionTransform_.scale.ToJson();

	JsonAdapter::Save("BossEnemy/Item/protrusion.json", data);
}
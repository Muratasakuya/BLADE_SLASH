#include "FieldBoundary.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Collision/CollisionDebugDraw.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	FieldBoundary classMethods
//============================================================================

void FieldBoundary::Init() {

	// json適用
	ApplyJson();

	// 1度だけ更新
	UpdateAllCollisionBody();
}

void FieldBoundary::SetPushBackTarget(Player* player, BossEnemy* bossEnemy) {

	player_ = nullptr;
	player_ = player;

	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;

	for (const auto& collision : collisions_) {

		collision->SetPushBackTarget(player_, bossEnemy_);
	}
	hasPrevPlayerPos_ = false;
	hasPrevBossPos_ = false;
}

void FieldBoundary::UpdateAllCollisionBody() {

	// 全ての衝突を更新
	for (const auto& collision : collisions_) {

		collision->Update();
	}

	// カプセル押し戻し処理
	if (useManualCapsuleSolver_) {

		SolveTargetsByCapsule();
	}
}

void FieldBoundary::ControlQuadPlayArea() {

	// 座標を制限する
	float clampSize = moveClampLength_ / 2.0f;

	// プレイヤー
	Vector3 translation = player_->GetTranslation();
	translation.x = std::clamp(translation.x, -clampSize, clampSize);
	translation.z = std::clamp(translation.z, -clampSize, clampSize);
	player_->SetTranslation(translation);

	// 敵
	translation = bossEnemy_->GetTranslation();
	translation.x = std::clamp(translation.x, -clampSize, clampSize);
	translation.z = std::clamp(translation.z, -clampSize, clampSize);
	bossEnemy_->SetTranslation(translation);

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// 枠線
	LineRenderer::GetInstance()->DrawSquare(moveClampLength_,
		Vector3(0.0f, 2.0f, 0.0f), Color::Yellow());
#endif
}

void FieldBoundary::ControlPushBack() {

	// カプセル押し戻し処理
	if (useManualCapsuleSolver_) {

		SolveTargetsByCapsule();
	}

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// 衝突カプセルデバッグ描画
	if (debugDrawCollision_ && useManualCapsuleSolver_) {

		CollisionDebugDraw::DrawCapsule(CollisionExt::MakeWorldCapsule(
			player_->GetTranslation(), playerCapsule_), Color::Yellow());
		CollisionDebugDraw::DrawCapsule(CollisionExt::MakeWorldCapsule(
			bossEnemy_->GetTranslation(), bossCapsule_), Color::Yellow());
	}
#endif
}

void FieldBoundary::SolveTargetsByCapsule() {

	if (collisions_.empty()) {
		return;
	}

	std::vector<CollisionShape::AABB> walls;
	walls.reserve(collisions_.size());
	for (auto& collision : collisions_) {

		walls.push_back(collision->GetWallAABB());
	}

	// 押し戻し解決
	auto SolvePush = [&](auto* actor, Vector3& prevPos, bool& hasPrev,
		const CollisionExt::CapsuleSettings& capsule, const Color& color) {

			if (!actor) {
				return;
			}

			Vector3 desired = actor->GetTranslation();
			if (!hasPrev) {

				prevPos = desired;
				hasPrev = true;
			}

			// 押し戻し解決
			Vector3 corrected = CollisionResolve::SolveCapsuleMoveAgainstAABBs(
				prevPos, desired, capsule, walls, solveSettings_);

			// 座標更新
			actor->SetTranslation(corrected);
			prevPos = corrected;
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
			if (debugDrawCollision_) {

				CollisionDebugDraw::DrawArrow(desired, corrected, color);
			}
#endif
		};

	// プレイヤーと敵の押し戻し処理
	SolvePush(player_, prevPlayerPos_, hasPrevPlayerPos_, playerCapsule_, Color::Yellow());
	SolvePush(bossEnemy_, prevBossPos_, hasPrevBossPos_, bossCapsule_, Color::Yellow());
}

void FieldBoundary::ImGui() {

	if (ImGui::Button("Save")) {
		SaveJson();
	}

	ImGui::SameLine();

	if (ImGui::Button("Add FieldWallCollision")) {

		auto collision = std::make_unique<FieldWallCollision>();
		collision->Init();
		collision->SetPushBackTarget(player_, bossEnemy_);
		collisions_.push_back(std::move(collision));
	}

	ImGui::DragFloat("moveClampLength", &moveClampLength_, 0.1f);

	for (uint32_t i = 0; i < collisions_.size(); ++i) {

		ImGui::PushID(static_cast<int>(i));
		if (ImGui::Button("Remove")) {

			collisions_.erase(collisions_.begin() + i);
			ImGui::PopID();
			--i;
			continue;
		}
		collisions_[i]->ImGui(i);
		collisions_[i]->Update();
		ImGui::PopID();
	}

	ImGui::SeparatorText("Capsule PushBack Settings");
	{
		bool prev = useManualCapsuleSolver_;
		ImGui::Checkbox("useManualCapsuleSolver", &useManualCapsuleSolver_);
		ImGui::Checkbox("debugDrawCollision", &debugDrawCollision_);

		// 切替時に前フレーム位置リセット
		if (prev != useManualCapsuleSolver_) {
			for (const auto& collision : collisions_) {

				collision->SetEnableCallbackPushBack(!useManualCapsuleSolver_);
			}
			hasPrevPlayerPos_ = false;
			hasPrevBossPos_ = false;
		}

		ImGui::Separator();

		ImGui::Text("Player Capsule");

		ImGui::DragFloat3("CenterOffset##Player", &playerCapsule_.centerOffset.x, 0.01f);
		ImGui::DragFloat("Radius##Player", &playerCapsule_.radius, 0.01f);
		ImGui::DragFloat("HalfHeight##Player", &playerCapsule_.halfHeight, 0.01f);

		ImGui::Separator();

		ImGui::Text("Boss Capsule");

		ImGui::DragFloat3("CenterOffset##Boss", &bossCapsule_.centerOffset.x);
		ImGui::DragFloat("Radius##Boss", &bossCapsule_.radius, 0.01f, 0.01f);
		ImGui::DragFloat("HalfHeight##Boss", &bossCapsule_.halfHeight, 0.01f);

		ImGui::Separator();

		ImGui::Text("Solve Settings");

		ImGui::DragFloat("MaxStepLength", &solveSettings_.maxStepLength, 0.01f);
		ImGui::DragInt("MaxResolveIterations", &solveSettings_.maxResolveIterations, 1.0f, 1);
		ImGui::DragFloat("SkinWidth", &solveSettings_.skinWidth, 0.0005f);

		ImGui::Separator();
	}

	// 値操作中にのみ更新
	UpdateAllCollisionBody();
}

void FieldBoundary::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Level/fieldCollisionCollection.json", data)) {
		return;
	}

	if (data.is_array()) {
		for (const auto& it : data) {

			auto collision = std::make_unique<FieldWallCollision>();
			collision->Init();
			collision->FromJson(it);
			collision->SetPushBackTarget(player_, bossEnemy_);
			collision->Update();
			collisions_.push_back(std::move(collision));
		}
	}

	// config
	{
		data.clear();
		if (JsonAdapter::LoadCheck("GameConfig/gameConfig.json", data)) {

			moveClampLength_ = JsonAdapter::GetValue<float>(data["playableArea"], "length");
		}
	}
	// カプセル
	{
		data.clear();
		if (JsonAdapter::LoadCheck("Level/capsuleCollisionSetting.json", data)) {

			playerCapsule_.FromJson(data.value("playerCapsule", Json()));
			bossCapsule_.FromJson(data.value("bossCapsule", Json()));
			solveSettings_.FromJson(data.value("solveSettings", Json()));
		}
	}
}

void FieldBoundary::SaveJson() {

	// collision
	{
		Json data = Json::array();
		for (const auto& collision : collisions_) {

			Json one;
			collision->ToJson(one);
			data.push_back(std::move(one));
		}
		JsonAdapter::Save("Level/FieldWallCollisionCollection.json", data);
	}
	// カプセル
	{
		Json data;

		playerCapsule_.ToJson(data["playerCapsule"]);
		bossCapsule_.ToJson(data["bossCapsule"]);
		solveSettings_.ToJson(data["solveSettings"]);

		JsonAdapter::Save("Level/capsuleCollisionSetting.json", data);
	}
}
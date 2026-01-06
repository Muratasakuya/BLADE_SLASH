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

namespace {

	static CollisionShape::Sphere MakeWorldSphere(const Vector3& actorPos,
		const FieldBoundary::SphereProxySettings& setting) {

		CollisionShape::Sphere out{};
		out.center = actorPos + setting.centerOffset;
		out.radius = setting.radius;
		return out;
	}

	static CollisionShape::Sphere MakeSolveSphere(const Vector3& actorPos,
		const FieldBoundary::SphereProxySettings& setting) {

		CollisionShape::Sphere out = MakeWorldSphere(actorPos, setting);
		if (setting.useXZOnly) {
			out.center.y = 0.0f;
		}
		return out;
	}

	static void ApplyXZOnlyPush(Vector3& push, bool xzOnly) {
		if (xzOnly) {

			push.y = 0.0f;
		}
	}
}

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

void FieldBoundary::ControlActorPushBack() {

	// アクター同士押し戻し処理
	if (enableActorPushBack_) {

		SolveTargetsBySpherePair();
	}
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

	// 壁とのプレイヤーと敵の押し戻し処理
	SolvePush(player_, prevPlayerPos_, hasPrevPlayerPos_, playerCapsule_, Color::Yellow());
	SolvePush(bossEnemy_, prevBossPos_, hasPrevBossPos_, bossCapsule_, Color::Yellow());
}

void FieldBoundary::SolveTargetsBySpherePair() {

	// アクター同士押し戻し解決
	const float skin = 0.002f;

	// 目標位置
	Vector3 desiredP = player_->GetTranslation();
	Vector3 desiredB = bossEnemy_->GetTranslation();

	// 押し戻すかどうか
	if (!hasPrevPlayerPosActor_) {

		prevPlayerPosActor_ = desiredP;
		hasPrevPlayerPosActor_ = true;
	}
	if (!hasPrevBossPosActor_) {

		prevBossPosActor_ = desiredB;
		hasPrevBossPosActor_ = true;
	}

	// 前フレームの位置
	Vector3 startP = prevPlayerPosActor_;
	Vector3 startB = prevBossPosActor_;
	// 移動量
	Vector3 deltaP = desiredP - startP;
	Vector3 deltaB = desiredB - startB;

	// 開始時の球
	CollisionShape::Sphere sp0 = MakeSolveSphere(startP, playerSphere_);
	CollisionShape::Sphere sb0 = MakeSolveSphere(startB, bossSphere_);

	// すでにのめり込んでいるなら押し出す
	{

		// のめり込みチェック
		CollisionResolve::PenetrationResult result = CollisionResolve::ComputeSphereVSSphere(
			MakeSolveSphere(desiredP, playerSphere_), MakeSolveSphere(desiredB, bossSphere_));
		// のめり込んでいたら押し出す
		if (result.isOverlapping) {

			// 押し出しベクトル
			Vector3 push = result.push;
			ApplyXZOnlyPush(push, playerSphere_.useXZOnly);

			// 半分ずつ押し出す
			desiredP += push * 0.5f;
			desiredB -= push * 0.5f;
			// 移動量再計算
			deltaP = desiredP - startP;
			deltaB = desiredB - startB;
		}
	}

	// のめり込みそうなときの処理
	float toi = 1.0f;
	Vector3 normal = Vector3::AnyInit(0.0f);
	// XZのみ判定するか
	const bool xzOnly = playerSphere_.useXZOnly || bossSphere_.useXZOnly;

	Vector3 dP = deltaP;
	Vector3 dB = deltaB;
	if (xzOnly) {
		dP.y = 0.0f;
		dB.y = 0.0f;
	}

	// 衝突判定
	bool hit = CollisionResolve::SweepSpherePairTOI(sp0, sb0, dP, dB, skin, xzOnly, toi, normal);

	float tStop = (std::max)(0.0f, (std::min)(1.0f, toi));
	Vector3 newP = startP + deltaP * tStop;
	Vector3 newB = startB + deltaB * tStop;
	if (hit) {

		// ほんの少し離す
		Vector3 separate = normal * skin;
		ApplyXZOnlyPush(separate, xzOnly);

		// 半分ずつ離す
		newP += separate * 0.5f;
		newB -= separate * 0.5f;

		// 座標更新
		player_->SetTranslation(newP);
		bossEnemy_->SetTranslation(newB);
		prevPlayerPosActor_ = newP;
		prevBossPosActor_ = newB;
		return;
	}

	// 当たらないならそのまま
	player_->SetTranslation(desiredP);
	bossEnemy_->SetTranslation(desiredB);
	prevPlayerPosActor_ = desiredP;
	prevBossPosActor_ = desiredB;

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	if (debugDrawActorCollision_) {

		// デバッグ球
		CollisionShape::Sphere drawP = MakeWorldSphere(newP, playerSphere_);
		CollisionShape::Sphere drawB = MakeWorldSphere(newB, bossSphere_);
		LineRenderer::GetInstance()->DrawSphere(6, drawP.radius, drawP.center, Color::Yellow());
		LineRenderer::GetInstance()->DrawSphere(6, drawB.radius, drawB.center, Color::Yellow());
	}
#endif
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

	ImGui::SeparatorText("Actor PushBack Settings");
	{
		ImGui::Checkbox("enableActorPushBack", &enableActorPushBack_);
		ImGui::Checkbox("debugDrawActorCollision", &debugDrawActorCollision_);

		ImGui::Text("Player SphereProxy");

		ImGui::DragFloat3("CenterOffset##PlayerSphere", &playerSphere_.centerOffset.x, 0.01f);
		ImGui::DragFloat("Radius##PlayerSphere", &playerSphere_.radius, 0.01f, 0.01f);
		ImGui::Checkbox("useXZOnly##PlayerSphere", &playerSphere_.useXZOnly);

		ImGui::Separator();

		ImGui::Text("Boss SphereProxy");

		ImGui::DragFloat3("CenterOffset##BossSphere", &bossSphere_.centerOffset.x, 0.01f);
		ImGui::DragFloat("Radius##BossSphere", &bossSphere_.radius, 0.01f, 0.01f);
		ImGui::Checkbox("useXZOnly##BossSphere", &bossSphere_.useXZOnly);
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

			playerSphere_.FromJson(data.value("playerSphere", Json()));
			bossSphere_.FromJson(data.value("bossSphere", Json()));
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

		playerSphere_.ToJson(data["playerSphere"]);
		bossSphere_.ToJson(data["bossSphere"]);

		JsonAdapter::Save("Level/capsuleCollisionSetting.json", data);
	}
}

void FieldBoundary::SphereProxySettings::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}
	centerOffset = Vector3::FromJson(data.value("centerOffset", Json()));
	radius = data.value("radius", radius);
	useXZOnly = data.value("useXZOnly", useXZOnly);
}

void FieldBoundary::SphereProxySettings::ToJson(Json& data) {

	data["centerOffset"] = centerOffset.ToJson();
	data["radius"] = radius;
	data["useXZOnly"] = useXZOnly;
}
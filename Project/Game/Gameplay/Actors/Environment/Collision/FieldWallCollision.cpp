#include "FieldWallCollision.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

// imgui
#include <imgui.h>

//============================================================================
//	FieldWallCollision classMethods
//============================================================================

void FieldWallCollision::Init() {

	// 衝突タイプ設定
	SakuEngine::CollisionBody* body = bodies_.emplace_back(SakuEngine::Collider::AddCollider(SakuEngine::CollisionShape::AABB().Default()));
	bodyOffsets_.emplace_back(SakuEngine::CollisionShape::AABB().Default());

	// タイプ設定
	body->SetType(ColliderType::Type_FieldWall);
	body->SetTargetType(ColliderType::Type_Player | ColliderType::Type_BossEnemy);

	// 初期設定
	SetIsChild(false);
}

void FieldWallCollision::SetPushBackTarget(Player* player, BossEnemy* bossEnemy) {

	player_ = nullptr;
	player_ = player;

	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;
}

SakuEngine::CollisionShape::AABB FieldWallCollision::GetWorldAABB() const {

	// オフセット分ずらしたAABBを取得
	const auto& local = std::get<SakuEngine::CollisionShape::AABB>(bodyOffsets_.front());
	SakuEngine::CollisionShape::AABB wall{};
	wall.center = local.center;
	wall.extent = local.extent;
	return wall;
}

void FieldWallCollision::Update() {

	// 衝突ボディを更新
	SakuEngine::Transform3D transform{};
	transform.scale = SakuEngine::Vector3::AnyInit(1.0f);
	transform.rotation = SakuEngine::Quaternion::Identity();
	transform.translation = SakuEngine::Vector3::AnyInit(0.0f);
	UpdateAllBodies(transform);
}

SakuEngine::CollisionShape::AABB FieldWallCollision::MakeAABBProxy(const SakuEngine::CollisionBody* other) {

	SakuEngine::CollisionShape::AABB proxy = SakuEngine::CollisionShape::AABB::Default();
	std::visit([&](const auto& shape) {

		// 形状をAABBに変換して返す
		using T = std::decay_t<decltype(shape)>;
		if constexpr (std::is_same_v<T, SakuEngine::CollisionShape::AABB>) {

			proxy = shape;
		} else if constexpr (std::is_same_v<T, SakuEngine::CollisionShape::Sphere>) {

			proxy.center = shape.center;
			proxy.extent = SakuEngine::Vector3::AnyInit(shape.radius);
		} else if constexpr (std::is_same_v<T, SakuEngine::CollisionShape::OBB>) {

			proxy.center = shape.center;
			const SakuEngine::Vector3 half = shape.size * 0.5f;
			proxy.extent = half;
		}
		}, other->GetShape());
	return proxy;
}

SakuEngine::Vector3 FieldWallCollision::ComputePushVector(
	const SakuEngine::CollisionShape::AABB& wall, const SakuEngine::CollisionShape::AABB& actor) {

	SakuEngine::Vector3 direction = actor.center - wall.center;
	SakuEngine::Vector3 overlap = (wall.extent + actor.extent) -
		SakuEngine::Vector3(SakuEngine::Math::AbsFloat(direction.x),
			SakuEngine::Math::AbsFloat(direction.y), SakuEngine::Math::AbsFloat(direction.z));
	if (overlap.x <= 0.0f || overlap.y <= 0.0f || overlap.z <= 0.0f) {
		return SakuEngine::Vector3::AnyInit(0.0f);
	}
	// 最小軸で押し戻す
	SakuEngine::Vector3 push{};
	if (overlap.x < overlap.y && overlap.x < overlap.z) {

		push.x = (direction.x >= 0.0f) ? overlap.x : -overlap.x;
	} else if (overlap.y < overlap.z) {

		push.y = (direction.y >= 0.0f) ? overlap.y : -overlap.y;
	} else {

		push.z = (direction.z >= 0.0f) ? overlap.z : -overlap.z;
	}
	return push;
}

void FieldWallCollision::OnCollisionStay(const SakuEngine::CollisionBody* collisionBody) {

	// 手動解決を使うなら従来押し戻しはOFF
	if (!enableCallbackPushBack_) {
		return;
	}

	// プレイヤーか敵が衝突したときに押し戻し処理を行う
	if ((collisionBody->GetType() & (ColliderType::Type_Player | ColliderType::Type_BossEnemy))
		!= ColliderType::Type_None) {

		const SakuEngine::CollisionShape::AABB wall = GetWorldAABB();
		const SakuEngine::CollisionShape::AABB actor = MakeAABBProxy(collisionBody);

		// 押し戻し方向を計算
		const SakuEngine::Vector3 push = ComputePushVector(wall, actor);
		if (push == SakuEngine::Vector3::AnyInit(0.0f)) {
			return;
		}

		// 押し戻し座標を設定
		if ((collisionBody->GetType() & ColliderType::Type_Player) != ColliderType::Type_None && player_) {

			player_->SetTranslation(player_->GetTranslation() + push);
		} else if ((collisionBody->GetType() & ColliderType::Type_BossEnemy) != ColliderType::Type_None && bossEnemy_) {

			bossEnemy_->SetTranslation(bossEnemy_->GetTranslation() + push);
		}
	}
}

void FieldWallCollision::ImGui(uint32_t index) {

	ImGui::PushID(index);

	ImGui::Checkbox("enableCallbackPushBack", &enableCallbackPushBack_);
	Collider::ImGui(192.0f);

	ImGui::PopID();
}

void FieldWallCollision::FromJson(const Json& data) {

	Collider::ApplyBodyOffset(data);
	enableCallbackPushBack_ = data.value("enableCallbackPushBack", false);
}

void FieldWallCollision::ToJson(Json& data) {

	Collider::SaveBodyOffset(data);
	data["enableCallbackPushBack"] = enableCallbackPushBack_;
}
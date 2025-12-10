#include "CollisionManager.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>

//============================================================================
//	CollisionManager classMethods
//============================================================================

namespace {

	template<class L, class R> struct NarrowPhase;
	using ShapeVariant = CollisionShape::Shapes;
	using CheckFunc = bool(*)(const ShapeVariant&, const ShapeVariant&);
	template<class L, class R>
	bool Call(const ShapeVariant& lhs, const ShapeVariant& rhs) {

		return NarrowPhase<L, R>::Test(std::get<L>(lhs), std::get<R>(rhs));
	}
#define NARROW(lhs, rhs, fn)                               \
template<> struct NarrowPhase<lhs, rhs> {              \
static bool Test(const lhs& a, const rhs& b)       \
{ return Collision::fn(a, b); }                    \
};

	NARROW(CollisionShape::Sphere, CollisionShape::Sphere, SphereToSphere)
		NARROW(CollisionShape::Sphere, CollisionShape::AABB, SphereToAABB)
		NARROW(CollisionShape::Sphere, CollisionShape::OBB, SphereToOBB)

		NARROW(CollisionShape::AABB, CollisionShape::Sphere, AABBToSphere)
		NARROW(CollisionShape::AABB, CollisionShape::AABB, AABBToAABB)
		NARROW(CollisionShape::AABB, CollisionShape::OBB, AABBToOBB)

		NARROW(CollisionShape::OBB, CollisionShape::Sphere, OBBToSphere)
		NARROW(CollisionShape::OBB, CollisionShape::AABB, OBBToAABB)
		NARROW(CollisionShape::OBB, CollisionShape::OBB, OBBToOBB)
#undef NARROW

		// 衝突する際のタイプテーブル
		constexpr CheckFunc DispatchTable[3][3] = {

			{ &Call<CollisionShape::Sphere, CollisionShape::Sphere>,
			  &Call<CollisionShape::Sphere, CollisionShape::AABB>,
			  &Call<CollisionShape::Sphere, CollisionShape::OBB> },

			{ &Call<CollisionShape::AABB,  CollisionShape::Sphere>,
			  &Call<CollisionShape::AABB,  CollisionShape::AABB>,
			  &Call<CollisionShape::AABB,  CollisionShape::OBB> },

			{ &Call<CollisionShape::OBB,   CollisionShape::Sphere>,
			  &Call<CollisionShape::OBB,   CollisionShape::AABB>,
			  &Call<CollisionShape::OBB,   CollisionShape::OBB> }
	};
}

CollisionManager* CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new CollisionManager();
	}
	return instance_;
}

void CollisionManager::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

CollisionBody* CollisionManager::AddCollisionBody(const CollisionShape::Shapes& shape) {

	CollisionBody* collider = new CollisionBody();
	collider->SetShape(shape);
	colliders_.emplace_back(collider);

	return collider;
}

void CollisionManager::RemoveCollisionBody(CollisionBody* collider) {

	auto itA = std::find(colliders_.begin(), colliders_.end(), collider);
	if (itA != colliders_.end()) {
		colliders_.erase(itA);
	}

	for (auto itB = preCollisions_.begin(); itB != preCollisions_.end();) {
		if (itB->first == collider || itB->second == collider) {

			itB = preCollisions_.erase(itB);
		} else {
			++itB;
		}
	}
}

void CollisionManager::ClearAllCollision() {

	colliders_.clear();
	preCollisions_.clear();
}

void CollisionManager::Update() {

	if (colliders_.empty()) {
		return;
	}

	std::unordered_set<std::pair<CollisionBody*, CollisionBody*>, pair_hash> currentCollisions;

	for (auto itA = colliders_.begin(); itA != colliders_.end(); ++itA) {

		auto itB = itA;
		++itB;

		for (; itB != colliders_.end(); ++itB) {

			CollisionBody* colliderA = *itA;
			CollisionBody* colliderB = *itB;

			auto hasPair = [](ColliderType a, ColliderType b) {
				using T = std::underlying_type_t<ColliderType>;
				return (static_cast<T>(a) & static_cast<T>(b)) != 0; };
			if (!hasPair(colliderA->GetTargetType(), colliderB->GetType()) &&
				!hasPair(colliderB->GetTargetType(), colliderA->GetType())) {
				continue;
			}

			if (IsColliding(colliderA, colliderB)) {
				currentCollisions.insert({ colliderA, colliderB });

				if (preCollisions_.find({ colliderA, colliderB }) == preCollisions_.end()) {

					colliderA->TriggerOnCollisionEnter(colliderB);
					colliderB->TriggerOnCollisionEnter(colliderA);
				} else {

					colliderA->TriggerOnCollisionStay(colliderB);
					colliderB->TriggerOnCollisionStay(colliderA);
				}
			}
		}
	}

	for (auto& collision : preCollisions_) {
		if (currentCollisions.find(collision) == currentCollisions.end()) {

			collision.first->TriggerOnCollisionExit(collision.second);
			collision.second->TriggerOnCollisionExit(collision.first);
		}
	}

	preCollisions_ = currentCollisions;

	// colliderの描画
	DrawCollider();
}

bool CollisionManager::IsColliding(CollisionBody* colliderA, CollisionBody* colliderB) {

	// 形状取得
	const auto& shapeA = colliderA->GetShape();
	const auto& shapeB = colliderB->GetShape();
	// 0:Sphere 1:AABB 2:OBB
	const size_t indexA = shapeA.index();
	const size_t indexB = shapeB.index();

	return DispatchTable[indexA][indexB](shapeA, shapeB);
}

void CollisionManager::DrawCollider() {

	LineRenderer* lineRenderer = LineRenderer::GetInstance();
	if (!lineRenderer) return;

	for (auto& collider : colliders_) {

		const auto& shape = collider->GetShape();
		Vector3 center = Vector3::AnyInit(0.0f);
		if (std::holds_alternative<CollisionShape::Sphere>(shape)) {

			center = std::get<CollisionShape::Sphere>(shape).center;
		} else if (std::holds_alternative<CollisionShape::AABB>(shape)) {

			center = std::get<CollisionShape::AABB>(shape).center;
		} else if (std::holds_alternative<CollisionShape::OBB>(shape)) {

			center = std::get<CollisionShape::OBB>(shape).center;
		}

		// コライダーが衝突しているかどうかを判定
		bool isColliding = false;
		for (const auto& collisionPair : preCollisions_) {
			if (collisionPair.first == collider || collisionPair.second == collider) {
				isColliding = true;
				break;
			}
		}

		// 衝突状態に応じた色を設定
		Color color = isColliding ? Color::Red() : Color::Convert(0x00ffffff);

		std::visit([&](const auto& shapeData) {
			using ShapeType = std::decay_t<decltype(shapeData)>;

			if constexpr (std::is_same_v<ShapeType, CollisionShape::Sphere>) {

				lineRenderer->DrawSphere(8, shapeData.radius, center, color);
			} else if constexpr (std::is_same_v<ShapeType, CollisionShape::AABB>) {

				lineRenderer->DrawAABB(shapeData.GetMin(), shapeData.GetMax(), color);
			} else if constexpr (std::is_same_v<ShapeType, CollisionShape::OBB>) {

				lineRenderer->DrawOBB(shapeData.center, shapeData.size, shapeData.rotate, color);
			}
			}, shape);
	}
}
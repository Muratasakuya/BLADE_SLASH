#include "CollisionBody.h"

//============================================================================
//	Collider classMethods
//============================================================================
#include <Engine/Collision/Collider.h>
#include <Engine/Core/Debug/Assert.h>

//============================================================================
//	CollisionBody  classMethods
//============================================================================

void CollisionBody::TriggerOnCollisionEnter(CollisionBody* collider) {

	if (onEnter_) {

		// 衝突した瞬間
		onEnter_(collider);
	}
}

void CollisionBody::TriggerOnCollisionStay(CollisionBody* collider) {

	if (onStay_) {

		// 衝突中
		onStay_(collider);
	}
}

void CollisionBody::TriggerOnCollisionExit(CollisionBody* collider) {

	if (onExit_) {

		// 離れた瞬間
		onExit_(collider);
	}
}

void CollisionBody::UpdateSphere(const CollisionShape::Sphere& sphere) {

	if (std::holds_alternative<CollisionShape::Sphere>(shape_)) {

		std::get<CollisionShape::Sphere>(shape_) = sphere;
	} else {

		ASSERT(FALSE, "collision shape is not 'sphere'");
	}
}

void CollisionBody::UpdateAABB(const CollisionShape::AABB& aabb) {

	if (std::holds_alternative<CollisionShape::AABB>(shape_)) {

		std::get<CollisionShape::AABB>(shape_) = aabb;
	} else {

		ASSERT(FALSE, "collision shape is not 'aabb'");
	}
}

void CollisionBody::UpdateOBB(const CollisionShape::OBB& obb) {

	if (std::holds_alternative<CollisionShape::OBB>(shape_)) {

		std::get<CollisionShape::OBB>(shape_) = obb;
	} else {

		ASSERT(FALSE, "collision shape is not 'obb'");
	}
}
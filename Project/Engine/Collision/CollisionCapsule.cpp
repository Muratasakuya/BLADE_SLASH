#include "CollisionCapsule.h"

using namespace SakuEngine;

//============================================================================
//	CollisionCapsule classMethods
//============================================================================

CollisionExt::Capsule CollisionExt::MakeWorldCapsule(const Vector3& actorTranslation, const CollisionExt::CapsuleSettings& settings) {

	// 正規化した上方向ベクトル
	Vector3 normalizedUp = settings.up.Normalize();
	float halfHeight = (std::max)(0.0f, settings.halfHeight);
	float radius = (std::max)(0.0f, settings.radius);
	Vector3 center = actorTranslation + settings.centerOffset;

	// カプセル生成
	CollisionExt::Capsule capsule{};
	capsule.pointA = center + normalizedUp * halfHeight;
	capsule.pointB = center - normalizedUp * halfHeight;
	capsule.radius = radius;
	return capsule;
}

void CollisionExt::CapsuleSettings::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}
	centerOffset = Vector3::FromJson(data["centerOffset"]);
	up = Vector3::FromJson(data["up"]);
	halfHeight = data.value("halfHeight", 1.0f);
	radius = data.value("radius", 0.4f);
}

void CollisionExt::CapsuleSettings::ToJson(Json& data) {

	data["centerOffset"] = centerOffset.ToJson();
	data["up"] = up.ToJson();
	data["halfHeight"] = halfHeight;
	data["radius"] = radius;
}
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/CollisionGeometry.h>
#include <Engine/Collision/CollisionCapsule.h>

namespace SakuEngine {

	//============================================================================
	//	CollisionResolve namespace
	//	押し戻し計算と、座標移動の解決処理を提供する
	//============================================================================
	namespace CollisionResolve {

		// 追加形状を含む汎用variant
		using Shape = std::variant<CollisionShape::Sphere, CollisionShape::AABB,
			CollisionShape::OBB, CollisionExt::Capsule>;

		// 押し戻し計算結果
		struct PenetrationResult {

			bool isOverlapping = false;
			Vector3 push = Vector3::AnyInit(0.0f);   // 移動に適用する押し戻し
			Vector3 normal = Vector3::AnyInit(0.0f); // 移動方向の法線
			float depth = 0.0f; // のめり込んだ深さ
		};

		//============================================================================
		//	衝突押し戻し判定、計算
		//============================================================================

		// Capsule VS AABB
		PenetrationResult ComputeCapsuleVSAABB(const CollisionExt::Capsule& capsule,
			const CollisionShape::AABB& aabb);

		// Sphere VS AABB
		PenetrationResult ComputeSphereVSAABB(const CollisionShape::Sphere& sphere,
			const CollisionShape::AABB& aabb);

		// AABB vs AABB
		PenetrationResult ComputeAABBVSAABB(const CollisionShape::AABB& moving,
			const CollisionShape::AABB& fixed);

		// Sphere VS Sphere
		PenetrationResult ComputeSphereVSSphere(const CollisionShape::Sphere& moving,
			const CollisionShape::Sphere& fixed);

		// Shape VS Shape
		PenetrationResult ComputePenetration(const Shape& moving, const Shape& fixed);

		// Sphere VS Sphereの時間的衝突判定
		bool SweepSpherePairTOI(const CollisionShape::Sphere& sphereA,
			const CollisionShape::Sphere& sphereB, const Vector3& deltaA, const Vector3& deltaB,
			float skinWidth, bool useXZOnly, float& outTOI, Vector3& outNormal);

		//============================================================================
		//	サブステップ移動解決
		//============================================================================

		// 解決設定
		struct SolveSettings {

			float maxStepLength = 0.0f;   // 最大ステップ長
			int maxResolveIterations = 6; // 1ステップあたりの解決反復回数
			float skinWidth = 0.001f;     // 面に貼り付き防止用の押し戻し

			// json
			void FromJson(const Json& data);
			void ToJson(Json& data);
		};

		// 移動前->移動後を、静的AABB群に対してカプセルとして解決し、補正後座標を返す
		Vector3 SolveCapsuleMoveAgainstAABBs(const Vector3& start,
			const Vector3& desired, const CollisionExt::CapsuleSettings& capsuleSettings,
			const std::vector<CollisionShape::AABB>& staticAabbs, const SolveSettings& solveSettings);
	}
}
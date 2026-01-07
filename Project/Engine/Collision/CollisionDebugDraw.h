#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/CollisionGeometry.h>
#include <Engine/Collision/CollisionCapsule.h>

namespace SakuEngine {

	//============================================================================
	// CollisionDebugDraw namespace
	// LineRendererを使ったデバッグ描画ユーティリティ
	//============================================================================
	namespace CollisionDebugDraw {

		void DrawAABB(const CollisionShape::AABB& aabb, const Color& color);
		void DrawCapsule(const CollisionExt::Capsule& capsule, const Color& color, int segments = 16);
		void DrawArrow(const Vector3& from, const Vector3& to, const Color& color, float headLength = 0.15f);
	}
}
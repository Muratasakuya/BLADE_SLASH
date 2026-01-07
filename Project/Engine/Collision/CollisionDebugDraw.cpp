#include "CollisionDebugDraw.h"

using namespace SakuEngine;

//============================================================================
// include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Config.h>
#include <Engine/MathLib/MathUtils.h>

// c++
#include <cmath>

namespace {

	// 安全に正規化(長さ0ベクトル対策)
	inline Vector3 SafeNormalize(const Vector3& v) {
		float len = v.Length();
		if (len <= Config::kEpsilon) {
			return Vector3(0.0f, 1.0f, 0.0f);
		}
		return v / len;
	}

	// 円を描画する
	inline void DrawCircle(const Vector3& center, const Vector3& axisX, const Vector3& axisY,
		float radius, const Color& color, int segments) {

		LineRenderer* lr = LineRenderer::GetInstance();
		segments = (std::max)(3, segments);
		float step = 2.0f * pi / static_cast<float>(segments);
		Vector3 prev = center + axisX * radius;

		for (int i = 1; i <= segments; ++i) {
			float a = step * static_cast<float>(i);
			Vector3 p = center + (axisX * std::cos(a) + axisY * std::sin(a)) * radius;
			lr->DrawLine3D(prev, p, color);
			prev = p;
		}
	}
}

//============================================================================

void CollisionDebugDraw::DrawAABB(const CollisionShape::AABB& aabb, const Color& color) {

	LineRenderer* lr = LineRenderer::GetInstance();
	Vector3 mn = aabb.GetMin();
	Vector3 mx = aabb.GetMax();

	Vector3 c000(mn.x, mn.y, mn.z);
	Vector3 c001(mn.x, mn.y, mx.z);
	Vector3 c010(mn.x, mx.y, mn.z);
	Vector3 c011(mn.x, mx.y, mx.z);
	Vector3 c100(mx.x, mn.y, mn.z);
	Vector3 c101(mx.x, mn.y, mx.z);
	Vector3 c110(mx.x, mx.y, mn.z);
	Vector3 c111(mx.x, mx.y, mx.z);

	lr->DrawLine3D(c000, c100, color);
	lr->DrawLine3D(c100, c101, color);
	lr->DrawLine3D(c101, c001, color);
	lr->DrawLine3D(c001, c000, color);

	lr->DrawLine3D(c010, c110, color);
	lr->DrawLine3D(c110, c111, color);
	lr->DrawLine3D(c111, c011, color);
	lr->DrawLine3D(c011, c010, color);

	lr->DrawLine3D(c000, c010, color);
	lr->DrawLine3D(c100, c110, color);
	lr->DrawLine3D(c101, c111, color);
	lr->DrawLine3D(c001, c011, color);
}

void CollisionDebugDraw::DrawCapsule(const CollisionExt::Capsule& capsule, const Color& color, int segments) {

	LineRenderer* lr = LineRenderer::GetInstance();
	segments = (std::max)(6, segments);

	Vector3 a = capsule.pointA;
	Vector3 b = capsule.pointB;
	float r = capsule.radius;

	Vector3 axis = a - b;
	float axisLen = axis.Length();
	Vector3 up = (axisLen > Config::kEpsilon) ? (axis / axisLen) : Vector3(0.0f, 1.0f, 0.0f);

	Vector3 ref = (std::abs(up.y) < 0.99f) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
	Vector3 right = SafeNormalize(Vector3::Cross(ref, up));
	Vector3 forward = SafeNormalize(Vector3::Cross(up, right));

	DrawCircle(a, right, forward, r, color, segments);
	DrawCircle(b, right, forward, r, color, segments);

	DrawCircle(a, right, up, r, color, segments);
	DrawCircle(a, forward, up, r, color, segments);
	DrawCircle(b, right, up, r, color, segments);
	DrawCircle(b, forward, up, r, color, segments);

	float step = 2.0f * pi / static_cast<float>(segments);
	for (int i = 0; i < segments; ++i) {

		float ang = step * static_cast<float>(i);
		Vector3 offset = (right * std::cos(ang) + forward * std::sin(ang)) * r;
		lr->DrawLine3D(a + offset, b + offset, color);
	}

	lr->DrawLine3D(a, b, color);
}

void CollisionDebugDraw::DrawArrow(const Vector3& from, const Vector3& to, const Color& color, float headLength) {

	LineRenderer* lr = LineRenderer::GetInstance();
	lr->DrawLine3D(from, to, color);

	Vector3 dir = to - from;
	float len = dir.Length();
	if (len <= Config::kEpsilon) { return; }
	Vector3 n = dir / len;

	Vector3 ref = (std::abs(n.y) < 0.99f) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
	Vector3 right = SafeNormalize(Vector3::Cross(ref, n));
	Vector3 up = SafeNormalize(Vector3::Cross(n, right));

	Vector3 tip = to;
	Vector3 base = to - n * headLength;
	Vector3 p0 = base + right * (headLength * 0.5f);
	Vector3 p1 = base - right * (headLength * 0.5f);
	Vector3 p2 = base + up * (headLength * 0.5f);
	Vector3 p3 = base - up * (headLength * 0.5f);

	lr->DrawLine3D(tip, p0, color);
	lr->DrawLine3D(tip, p1, color);
	lr->DrawLine3D(tip, p2, color);
	lr->DrawLine3D(tip, p3, color);
}
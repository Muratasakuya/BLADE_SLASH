#include "CollisionResolution.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>

//============================================================================
//	CollisionResolution classMethods
//============================================================================

namespace {

	// 座標をAABB内にクランプ
	Vector3 ClampPointToAABB(const Vector3& p, const Vector3& bmin, const Vector3& bmax) {

		return Vector3(std::clamp(p.x, bmin.x, bmax.x), std::clamp(p.y, bmin.y, bmax.y), std::clamp(p.z, bmin.z, bmax.z));
	}

	// セグメントp0 + d * t上の点とAABBの距離二乗を評価
	float EvaluateDistanceSegmentPointToAABB(const Vector3& p0, const Vector3& d,
		float t, const Vector3& bmin, const Vector3& bmax, Vector3* outSegPt, Vector3* outBoxPt) {

		Vector3 s = p0 + d * t;
		Vector3 c = ClampPointToAABB(s, bmin, bmax);
		if (outSegPt) {
			*outSegPt = s;
		}
		if (outBoxPt) {
			*outBoxPt = c;
		}

		Vector3 diff = s - c;
		return Vector3::Dot(diff, diff);
	}

	//----------------------------------------------------------------------------
	// ClosestPtSegmentAABB
	//----------------------------------------------------------------------------
	float ClosestPtSegmentAABB(const Vector3& p0, const Vector3& p1,
		const CollisionShape::AABB& aabb, float& outT, Vector3& outSegPt, Vector3& outBoxPt) {

		const Vector3 bmin = aabb.GetMin();
		const Vector3 bmax = aabb.GetMax();
		const Vector3 d = p1 - p0;

		std::array<float, 8> tsRaw{};
		int tsCount = 0;
		tsRaw[tsCount++] = 0.0f;
		tsRaw[tsCount++] = 1.0f;

		auto addT = [&](float t) {
			if (t < 0.0f || t > 1.0f) { return; }
			tsRaw[tsCount++] = t;
			};

		auto addAxis = [&](float p0i, float di, float minI, float maxI) {
			if (std::abs(di) < Config::kEpsilon) {
				return;
			}
			addT((minI - p0i) / di);
			addT((maxI - p0i) / di);
			};

		addAxis(p0.x, d.x, bmin.x, bmax.x);
		addAxis(p0.y, d.y, bmin.y, bmax.y);
		addAxis(p0.z, d.z, bmin.z, bmax.z);

		// sort + unique
		std::sort(tsRaw.begin(), tsRaw.begin() + tsCount);
		std::vector<float> ts;
		ts.reserve(static_cast<size_t>(tsCount));
		for (int i = 0; i < tsCount; ++i) {
			float t = std::clamp(tsRaw[i], 0.0f, 1.0f);
			if (ts.empty() || std::abs(ts.back() - t) > 1e-6f) {
				ts.push_back(t);
			}
		}
		if (ts.size() == 1) {
			ts.push_back(1.0f);
		}

		float bestT = 0.0f;
		Vector3 bestSeg = p0;
		Vector3 bestBox = ClampPointToAABB(p0, bmin, bmax);
		float bestSq = Vector3::Dot(bestSeg - bestBox, bestSeg - bestBox);

		// endpoints
		{
			Vector3 s, c;
			float sq0 = EvaluateDistanceSegmentPointToAABB(p0, d, 0.0f, bmin, bmax, &s, &c);
			if (sq0 < bestSq) { bestSq = sq0; bestT = 0.0f; bestSeg = s; bestBox = c; }
			float sq1 = EvaluateDistanceSegmentPointToAABB(p0, d, 1.0f, bmin, bmax, &s, &c);
			if (sq1 < bestSq) { bestSq = sq1; bestT = 1.0f; bestSeg = s; bestBox = c; }
		}

		for (size_t i = 0; i + 1 < ts.size(); ++i) {
			float ta = ts[i];
			float tb = ts[i + 1];
			if (tb - ta < 1e-6f) { continue; }

			float tMid = (ta + tb) * 0.5f;
			Vector3 pMid = p0 + d * tMid;

			struct AxisState { bool active = false; float bound = 0.0f; };
			AxisState ax[3];

			auto classify = [&](float v, float minI, float maxI, AxisState& out) {
				if (v < minI) { out.active = true; out.bound = minI; } else if (v > maxI) { out.active = true; out.bound = maxI; } else { out.active = false; out.bound = 0.0f; }
				};

			classify(pMid.x, bmin.x, bmax.x, ax[0]);
			classify(pMid.y, bmin.y, bmax.y, ax[1]);
			classify(pMid.z, bmin.z, bmax.z, ax[2]);

			float a = 0.0f;
			float b = 0.0f;
			float c = 0.0f;

			auto addTerm = [&](float p0i, float di, const AxisState& st) {
				if (!st.active) { return; }
				float k = (p0i - st.bound);
				a += di * di;
				b += 2.0f * di * k;
				c += k * k;
				};

			addTerm(p0.x, d.x, ax[0]);
			addTerm(p0.y, d.y, ax[1]);
			addTerm(p0.z, d.z, ax[2]);

			if (a == 0.0f && b == 0.0f && c == 0.0f) {
				bestSq = 0.0f;
				bestT = tMid;
				bestSeg = pMid;
				bestBox = pMid;
				break;
			}

			float tCand = ta;
			if (a > Config::kEpsilon) {
				tCand = -b / (2.0f * a);
				tCand = std::clamp(tCand, ta, tb);
			}

			Vector3 s, cpt;
			float sq = EvaluateDistanceSegmentPointToAABB(p0, d, tCand, bmin, bmax, &s, &cpt);
			if (sq < bestSq) { bestSq = sq; bestT = tCand; bestSeg = s; bestBox = cpt; }

			sq = EvaluateDistanceSegmentPointToAABB(p0, d, ta, bmin, bmax, &s, &cpt);
			if (sq < bestSq) { bestSq = sq; bestT = ta; bestSeg = s; bestBox = cpt; }
			sq = EvaluateDistanceSegmentPointToAABB(p0, d, tb, bmin, bmax, &s, &cpt);
			if (sq < bestSq) { bestSq = sq; bestT = tb; bestSeg = s; bestBox = cpt; }
		}

		outT = bestT;
		outSegPt = bestSeg;
		outBoxPt = bestBox;
		return bestSq;
	}

	void ExpandedAABBMinMax(const CollisionShape::AABB& aabb, float radius,
		Vector3& outMin, Vector3& outMax) {

		Vector3 bmin = aabb.GetMin();
		Vector3 bmax = aabb.GetMax();
		Vector3 r = Vector3::AnyInit(radius);
		outMin = bmin - r;
		outMax = bmax + r;
	}
}

CollisionResolve::PenetrationResult SakuEngine::CollisionResolve::ComputeCapsuleVSAABB(
	const CollisionExt::Capsule& capsule, const CollisionShape::AABB& aabb) {

	PenetrationResult result{};

	float t = 0.0f;
	Vector3 segPt, boxPt;
	float distSq = ClosestPtSegmentAABB(capsule.pointA, capsule.pointB, aabb, t, segPt, boxPt);

	float r = capsule.radius;
	float rSq = r * r;
	if (distSq > rSq) { return result; }

	float dist = std::sqrt((std::max)(0.0f, distSq));

	if (dist > Config::kEpsilon) {
		Vector3 n = (segPt - boxPt) / dist;
		float depth = (r - dist);
		result.isOverlapping = depth > 0.0f;
		result.depth = depth;
		result.normal = n;
		result.push = n * depth;
		return result;
	}

	// dist==0：セグメントがAABBに刺さってる → 法線が決められない
	Vector3 bminExp, bmaxExp;
	ExpandedAABBMinMax(aabb, r, bminExp, bmaxExp);

	Vector3 smin(
		(std::min)(capsule.pointA.x, capsule.pointB.x),
		(std::min)(capsule.pointA.y, capsule.pointB.y),
		(std::min)(capsule.pointA.z, capsule.pointB.z));
	Vector3 smax(
		(std::max)(capsule.pointA.x, capsule.pointB.x),
		(std::max)(capsule.pointA.y, capsule.pointB.y),
		(std::max)(capsule.pointA.z, capsule.pointB.z));

	auto axisPush = [&](float segMin, float segMax, float boxMin, float boxMax) -> float {
		float pushNeg = boxMin - segMax; // <= 0
		float pushPos = boxMax - segMin; // >= 0
		return (std::abs(pushNeg) < std::abs(pushPos)) ? pushNeg : pushPos;
		};

	float px = axisPush(smin.x, smax.x, bminExp.x, bmaxExp.x);
	float py = axisPush(smin.y, smax.y, bminExp.y, bmaxExp.y);
	float pz = axisPush(smin.z, smax.z, bminExp.z, bmaxExp.z);

	float ax = std::abs(px);
	float ay = std::abs(py);
	float az = std::abs(pz);

	Vector3 push = Vector3::AnyInit(0.0f);
	Vector3 normal = Vector3::AnyInit(0.0f);
	float depth = 0.0f;

	if (ax <= ay && ax <= az) {
		push.x = px;
		normal.x = (px >= 0.0f) ? 1.0f : -1.0f;
		depth = ax;
	} else if (ay <= az) {
		push.y = py;
		normal.y = (py >= 0.0f) ? 1.0f : -1.0f;
		depth = ay;
	} else {
		push.z = pz;
		normal.z = (pz >= 0.0f) ? 1.0f : -1.0f;
		depth = az;
	}

	result.isOverlapping = depth > 0.0f;
	result.depth = depth;
	result.normal = normal;
	result.push = push;
	return result;
}

CollisionResolve::PenetrationResult CollisionResolve::ComputeSphereVSAABB(
	const CollisionShape::Sphere& sphere,
	const CollisionShape::AABB& aabb) {

	CollisionResolve::PenetrationResult result{};

	Vector3 bmin = aabb.GetMin();
	Vector3 bmax = aabb.GetMax();
	Vector3 closest = Vector3(
		std::clamp(sphere.center.x, bmin.x, bmax.x),
		std::clamp(sphere.center.y, bmin.y, bmax.y),
		std::clamp(sphere.center.z, bmin.z, bmax.z));
	Vector3 diff = sphere.center - closest;
	float distSq = Vector3::Dot(diff, diff);
	float r = sphere.radius;
	if (distSq > r * r) { return result; }

	float dist = std::sqrt((std::max)(0.0f, distSq));
	if (dist > Config::kEpsilon) {
		Vector3 n = diff / dist;
		float depth = r - dist;
		result.isOverlapping = depth > 0.0f;
		result.depth = depth;
		result.normal = n;
		result.push = n * depth;
		return result;
	}

	Vector3 bminExp, bmaxExp;
	ExpandedAABBMinMax(aabb, r, bminExp, bmaxExp);

	float left = sphere.center.x - bminExp.x;
	float right = bmaxExp.x - sphere.center.x;
	float down = sphere.center.y - bminExp.y;
	float up = bmaxExp.y - sphere.center.y;
	float back = sphere.center.z - bminExp.z;
	float front = bmaxExp.z - sphere.center.z;

	float m = left;
	Vector3 normal(-1.0f, 0.0f, 0.0f);
	Vector3 push(-left, 0.0f, 0.0f);

	auto tryAxis = [&](float v, const Vector3& n, const Vector3& p) {
		if (v < m) { m = v; normal = n; push = p; }
		};

	tryAxis(right, Vector3(1.0f, 0.0f, 0.0f), Vector3(right, 0.0f, 0.0f));
	tryAxis(down, Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, -down, 0.0f));
	tryAxis(up, Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, up, 0.0f));
	tryAxis(back, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -back));
	tryAxis(front, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, front));

	result.isOverlapping = m > 0.0f;
	result.depth = m;
	result.normal = normal;
	result.push = push;
	return result;
}

CollisionResolve::PenetrationResult CollisionResolve::ComputeAABBVSAABB(
	const CollisionShape::AABB& moving,
	const CollisionShape::AABB& fixed) {

	CollisionResolve::PenetrationResult result{};

	Vector3 d = moving.center - fixed.center;
	Vector3 overlap = (moving.extent + fixed.extent) - Vector3(
		std::abs(d.x),
		std::abs(d.y),
		std::abs(d.z));

	if (overlap.x <= 0.0f || overlap.y <= 0.0f || overlap.z <= 0.0f) {
		return result;
	}

	Vector3 push = Vector3::AnyInit(0.0f);
	Vector3 normal = Vector3::AnyInit(0.0f);
	float depth = 0.0f;

	if (overlap.x <= overlap.y && overlap.x <= overlap.z) {
		push.x = (d.x >= 0.0f) ? overlap.x : -overlap.x;
		normal.x = (push.x >= 0.0f) ? 1.0f : -1.0f;
		depth = std::abs(push.x);
	} else if (overlap.y <= overlap.z) {
		push.y = (d.y >= 0.0f) ? overlap.y : -overlap.y;
		normal.y = (push.y >= 0.0f) ? 1.0f : -1.0f;
		depth = std::abs(push.y);
	} else {
		push.z = (d.z >= 0.0f) ? overlap.z : -overlap.z;
		normal.z = (push.z >= 0.0f) ? 1.0f : -1.0f;
		depth = std::abs(push.z);
	}

	result.isOverlapping = depth > 0.0f;
	result.depth = depth;
	result.normal = normal;
	result.push = push;
	return result;
}

CollisionResolve::PenetrationResult CollisionResolve::ComputeSphereVSSphere(
	const CollisionShape::Sphere& moving, const CollisionShape::Sphere& fixed) {

	PenetrationResult result{};

	Vector3 d = moving.center - fixed.center;
	float distSq = Vector3::Dot(d, d);
	float r = moving.radius + fixed.radius;

	if (distSq >= r * r) {
		return result;
	}

	float dist = std::sqrt((std::max)(0.0f, distSq));
	Vector3 n = Vector3(1.0f, 0.0f, 0.0f);
	if (dist > Config::kEpsilon) {
		n = d / dist;
	}

	float depth = r - dist;
	result.isOverlapping = depth > 0.0f;
	result.depth = depth;
	result.normal = n;          // fixed→moving
	result.push = n * depth;    // movingを押し出す
	return result;
}

CollisionResolve::PenetrationResult CollisionResolve::ComputePenetration(
	const Shape& moving, const Shape& fixed) {

	PenetrationResult out{};

	std::visit([&](const auto& m, const auto& f) {
		using M = std::decay_t<decltype(m)>;
		using F = std::decay_t<decltype(f)>;

		if constexpr (std::is_same_v<M, CollisionExt::Capsule> && std::is_same_v<F, CollisionShape::AABB>) {
			out = ComputeCapsuleVSAABB(m, f);
		} else if constexpr (std::is_same_v<M, CollisionShape::Sphere> && std::is_same_v<F, CollisionShape::AABB>) {
			out = ComputeSphereVSAABB(m, f);
		} else if constexpr (std::is_same_v<M, CollisionShape::AABB> && std::is_same_v<F, CollisionShape::AABB>) {
			out = ComputeAABBVSAABB(m, f);
		} else {
			out = PenetrationResult{};
		}
		}, moving, fixed);

	return out;
}

bool SakuEngine::CollisionResolve::SweepSpherePairTOI(const CollisionShape::Sphere& sphereA,
	const CollisionShape::Sphere& sphereB, const Vector3& deltaA, const Vector3& deltaB,
	float skinWidth, bool useXZOnly, float& outTOI, Vector3& outNormal) {

	Vector3 p = sphereA.center - sphereB.center;
	Vector3 v = deltaA - deltaB;
	float R = sphereA.radius + sphereB.radius + skinWidth;

	if (useXZOnly) {
		p.y = 0.0f;
		v.y = 0.0f;
	}

	float c = Vector3::Dot(p, p) - R * R;

	// 既に重なっている
	if (c <= 0.0f) {
		outTOI = 0.0f;
		float len = p.Length();
		outNormal = (len > Config::kEpsilon) ? (p / len) : Vector3(1.0f, 0.0f, 0.0f);
		return true;
	}

	float a = Vector3::Dot(v, v);
	// 相対速度ほぼゼロ
	if (a <= Config::kEpsilon) {
		return false;
	}

	float b = 2.0f * Vector3::Dot(p, v);
	float disc = b * b - 4.0f * a * c;
	if (disc < 0.0f) {
		return false;
	}

	float sqrtDisc = std::sqrt(disc);
	float t = (-b - sqrtDisc) / (2.0f * a);
	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	Vector3 hitRel = p + v * t;
	float hitLen = hitRel.Length();
	outNormal = (hitLen > Config::kEpsilon) ? (hitRel / hitLen) : Vector3(1.0f, 0.0f, 0.0f);
	outTOI = t;
	return true;
}

Vector3 CollisionResolve::SolveCapsuleMoveAgainstAABBs(
	const Vector3& start,
	const Vector3& desired,
	const CollisionExt::CapsuleSettings& capsuleSettings,
	const std::vector<CollisionShape::AABB>& staticAabbs,
	const SolveSettings& solveSettings) {

	Vector3 pos = start;
	Vector3 delta = desired - start;
	float dist = delta.Length();

	float maxStep = solveSettings.maxStepLength;
	if (maxStep <= 0.0f) {
		maxStep = ((std::max))(0.05f, capsuleSettings.radius * 0.5f);
	}

	int steps = 1;
	if (dist > Config::kEpsilon) {
		steps = (std::max)(1, static_cast<int>(std::ceil(dist / maxStep)));
	}
	Vector3 stepDelta = delta / static_cast<float>(steps);

	for (int stepIndex = 0; stepIndex < steps; ++stepIndex) {
		pos += stepDelta;

		for (int iter = 0; iter < solveSettings.maxResolveIterations; ++iter) {
			bool any = false;
			CollisionExt::Capsule cap = CollisionExt::MakeWorldCapsule(pos, capsuleSettings);

			for (const auto& wall : staticAabbs) {
				PenetrationResult pen = ComputeCapsuleVSAABB(cap, wall);
				if (!pen.isOverlapping) { continue; }

				Vector3 push = pen.push;
				if (solveSettings.skinWidth > 0.0f) {
					push += pen.normal * solveSettings.skinWidth;
				}

				pos += push;
				any = true;

				cap = CollisionExt::MakeWorldCapsule(pos, capsuleSettings);
			}

			if (!any) { break; }
		}
	}

	return pos;
}

void CollisionResolve::SolveSettings::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	maxStepLength = data.value("maxStepLength", maxStepLength);
	maxResolveIterations = data.value("maxResolveIterations", maxResolveIterations);
	skinWidth = data.value("skinWidth", skinWidth);
}

void CollisionResolve::SolveSettings::ToJson(Json& data) {

	data["maxStepLength"] = maxStepLength;
	data["maxResolveIterations"] = maxResolveIterations;
	data["skinWidth"] = skinWidth;
}
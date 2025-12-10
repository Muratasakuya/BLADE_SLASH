#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cmath>
#include <math.h>
#include <vector>
#include <cassert>

// json
#include <Externals/nlohmann/json.hpp>
// using
using Json = nlohmann::json;

namespace SakuEngine {

	// front
	template <typename tValue>
	struct Keyframe;
	class Matrix4x4;

	//============================================================================
	//	Vector3 class
	//============================================================================
	class Vector3 final {
	public:

		float x, y, z;

		Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
		Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

		//--------- operators ----------------------------------------------------
		
		Vector3 operator+(const Vector3& other) const;
		Vector3 operator-(const Vector3& other) const;
		Vector3 operator*(const Vector3& other) const;
		Vector3 operator/(const Vector3& other) const;

		Vector3& operator+=(const Vector3& v);
		Vector3& operator-=(const Vector3& v);
		Vector3& operator*=(const Vector3& v);

		Vector3 operator-(float scalar) const;
		Vector3 operator*(float scalar) const;
		Vector3 operator/(float scalar) const;

		Vector3& operator+=(float scalar);
		Vector3& operator-=(float scalar);
		Vector3& operator*=(float scalar);
		Vector3& operator/=(float scalar);

		Vector3 operator-() const;

		bool operator==(const Vector3& other) const;
		bool operator!=(const Vector3& other) const;

		bool operator>=(const Vector3& other) const;
		bool operator<=(const Vector3& other) const;

		//----------- json -------------------------------------------------------

		Json ToJson() const;
		static SakuEngine::Vector3 FromJson(const Json& data);

		//--------- functions ----------------------------------------------------

		void Init();

		float Length() const;

		Vector3 Normalize() const;

		static SakuEngine::Vector3 AnyInit(float value);

		static SakuEngine::Vector3 RotateVector(const SakuEngine::Vector3& baseDirection, float angleRad);

		static SakuEngine::Vector3 Cross(const SakuEngine::Vector3& v0, const SakuEngine::Vector3& v1);
		static float Dot(const SakuEngine::Vector3& v0, const SakuEngine::Vector3& v1);

		static float Length(const SakuEngine::Vector3& v);

		static SakuEngine::Vector3 Normalize(const SakuEngine::Vector3& v);

		static SakuEngine::Vector3 CalculateValue(const std::vector<Keyframe<Vector3>>& keyframes, float time);

		static SakuEngine::Vector3 Lerp(const SakuEngine::Vector3& v0, const SakuEngine::Vector3& v1, float t);

		static SakuEngine::Vector3 Reflect(const SakuEngine::Vector3& input, const SakuEngine::Vector3& normal);

		static SakuEngine::Vector3 Transform(const SakuEngine::Vector3& v, const Matrix4x4& matrix);

		static SakuEngine::Vector3 TransferNormal(const SakuEngine::Vector3& v, const Matrix4x4& m);

		static SakuEngine::Vector3 Projection(const SakuEngine::Vector3& v0, const SakuEngine::Vector3& v1);

		static SakuEngine::Vector3 ClosestPointOnLine(const SakuEngine::Vector3& point, const SakuEngine::Vector3& origin, const SakuEngine::Vector3& diff);
		static SakuEngine::Vector3 ClosestPointOnRay(const SakuEngine::Vector3& point, const SakuEngine::Vector3& origin, const SakuEngine::Vector3& diff);
		static SakuEngine::Vector3 ClosestPointOnSegment(const SakuEngine::Vector3& point, const SakuEngine::Vector3& origin, const SakuEngine::Vector3& diff);
	};

	Vector3 operator*(float scalar, const Vector3& v);
	Vector3 operator-(float scalar, const Vector3& v);
	Vector3 operator/(float scalar, const Vector3& v);
}; // SakuEngine

#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/Base/BaseLineRenderer.h>

namespace SakuEngine {

	//============================================================================
	//	LineRenderer3D class
	//	3Dライン描画を担当するクラス
	//============================================================================
	class LineRenderer3D :
		public BaseLineRenderer {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		LineRenderer3D() = default;
		~LineRenderer3D() = default;

		//========================================================================
		//	描画関数群
		//========================================================================

		// グリッド線を描画する
		void DrawGrid(int division, float gridSize, const Color& color, LineType type = LineType::None);
		// 軸を描画
		template <typename T>
		void DrawAxis(float length, const Vector3& centerPos, const T& rotation, LineType type = LineType::None);
		// 端点に球マーカーを付けた線分を描画する
		void DrawSegment(int division, float radius, const Vector3& pointA,
			const Vector3& pointB, const Color& color, LineType type = LineType::None);

		//--------- プリミティブ形状 ------------------------------------------------

		// 緯度経度分割のワイヤ球を描画
		void DrawSphere(int division, float radius, const Vector3& centerPos,
			const Color& color, LineType type = LineType::None);
		// AABBの枠線を描画
		void DrawAABB(const Vector3& min, const Vector3& max,
			const Color& color, LineType type = LineType::None);
		// XZ平面の円を描画
		void DrawCircle(int division, float radius, const Vector3& center,
			const Color& color, LineType type = LineType::None);
		// 方向ベクトルを中心に扇形(弧)を描画
		void DrawArc(int division, float radius, float halfAngle, const Vector3& center,
			const Vector3& direction, const Color& color, LineType type = LineType::None);
		// 正方形を描画
		void DrawSquare(float length, const Vector3& center,
			const Color& color, LineType type = LineType::None);
		// 矩形を描画
		void DrawRect(const Vector2& size, const Vector3& center,
			const Color& color, LineType type = LineType::None);

		// N角形の枠線を描く(3〜12)
		template <typename T>
		void DrawPolygon(int polygonCount, const Vector3& centerPos, float scale,
			const T& rotation, const Color& color, LineType type = LineType::None);
		// 上半球のワイヤ描画
		template <typename T>
		void DrawHemisphere(int division, float radius, const Vector3& centerPos,
			const T& rotation, const Color& color, LineType type = LineType::None);
		// OBBの枠線を描画
		template <typename T>
		void DrawOBB(const Vector3& centerPos, const Vector3& size,
			const T& rotation, const Color& color, LineType type = LineType::None);
		// 円柱/円錐のワイヤ(上下円と側面)を描画
		template <typename T>
		void DrawCone(int division, float baseRadius, float topRadius, float height,
			const Vector3& centerPos, const T& rotation, const Color& color, LineType type = LineType::None);

		//--------- accessor -----------------------------------------------------

		LineDimension GetLineDimension() const { return LineDimension::Line3D; }
	};

	//============================================================================
	//	LineRenderer3D templateMethods
	//============================================================================

	template<typename T>
	inline void LineRenderer3D::DrawAxis(float length, const Vector3& centerPos, const T& rotation, LineType type) {

		// X軸(赤)Y軸(青)Z軸(緑)
		// 回転
		Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
		if constexpr (std::is_same_v<T, Vector3>) {

			rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Quaternion>) {

			rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Matrix4x4>) {

			rotationMatrix = rotation;
		}

		// 基準軸を回転させて、ワールド空間の軸方向にする
		Vector3 xDirection = Vector3::TransferNormal(Direction::Get(Direction3D::Right), rotationMatrix).Normalize();
		Vector3 yDirection = Vector3::TransferNormal(Direction::Get(Direction3D::Up), rotationMatrix).Normalize();
		Vector3 zDirection = Vector3::TransferNormal(Direction::Get(Direction3D::Forward), rotationMatrix).Normalize();

		BaseLineRenderer::DrawLine(centerPos, centerPos + xDirection * length, Color::Red(), type);
		BaseLineRenderer::DrawLine(centerPos, centerPos + yDirection * length, Color::Blue(), type);
		BaseLineRenderer::DrawLine(centerPos, centerPos + zDirection * length, Color::Green(), type);
	}

	template<typename T>
	inline void LineRenderer3D::DrawPolygon(int polygonCount, const Vector3& centerPos,
		float scale, const T& rotation, const Color& color, LineType type) {

		if (polygonCount < 3 || polygonCount > 12) {
			return;
		}

		std::vector<Vector3> vertices;
		vertices.reserve(polygonCount);

		// 回転
		Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
		if constexpr (std::is_same_v<T, Vector3>) {

			rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Quaternion>) {

			rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Matrix4x4>) {

			rotationMatrix = rotation;
		}

		// 各頂点を計算
		for (int i = 0; i < polygonCount; ++i) {
			float angle = 2.0f * pi * static_cast<float>(i) / static_cast<float>(polygonCount);
			Vector3 localPos = { std::cos(angle) * scale, 0.0f, std::sin(angle) * scale };
			Vector3 worldPos = rotationMatrix.TransformPoint(localPos) + centerPos;
			vertices.push_back(worldPos);
		}

		// 線を描画
		for (int i = 0; i < polygonCount; ++i) {

			const Vector3& start = vertices[i];
			const Vector3& end = vertices[(i + 1) % polygonCount];
			BaseLineRenderer::DrawLine(start, end, color, type);
		}
	}

	template<typename T>
	inline void LineRenderer3D::DrawHemisphere(int division, float radius, const Vector3& centerPos,
		const T& rotation, const Color& color, LineType type) {

		const float kLatEvery = (pi / 2.0f) / division; // 緯度
		const float kLonEvery = 2.0f * pi / division;   // 経度

		auto calculatePoint = [&](float lat, float lon) -> Vector3 {
			return {
				radius * std::cos(lat) * std::cos(lon),
				radius * std::sin(lat),
				radius * std::cos(lat) * std::sin(lon)
			};
			};

		Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
		if constexpr (std::is_same_v<T, Vector3>) {

			rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Quaternion>) {

			rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Matrix4x4>) {

			rotationMatrix = rotation;
		}

		for (int latIndex = 0; latIndex < division; ++latIndex) {
			float lat = kLatEvery * latIndex;
			for (int lonIndex = 0; lonIndex < division; ++lonIndex) {
				float lon = lonIndex * kLonEvery;

				Vector3 pointA = calculatePoint(lat, lon);
				Vector3 pointB = calculatePoint(lat + kLatEvery, lon);
				Vector3 pointC = calculatePoint(lat, lon + kLonEvery);

				pointA = rotationMatrix.TransformPoint(pointA) + centerPos;
				pointB = rotationMatrix.TransformPoint(pointB) + centerPos;
				pointC = rotationMatrix.TransformPoint(pointC) + centerPos;

				BaseLineRenderer::DrawLine(pointA, pointB, color, type);
				BaseLineRenderer::DrawLine(pointA, pointC, color, type);
			}
		}
	}

	template<typename T>
	inline void LineRenderer3D::DrawOBB(const Vector3& centerPos, const Vector3& size,
		const T& rotation, const Color& color, LineType type) {

		const uint32_t vertexNum = 8;

		Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
		if constexpr (std::is_same_v<T, Vector3>) {

			rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Quaternion>) {

			rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Matrix4x4>) {

			rotationMatrix = rotation;
		}

		Vector3 vertices[vertexNum];
		Vector3 halfSizeX = Vector3::Transform(Direction::Get(Direction3D::Right), rotationMatrix) * size.x;
		Vector3 halfSizeY = Vector3::Transform(Direction::Get(Direction3D::Up), rotationMatrix) * size.y;
		Vector3 halfSizeZ = Vector3::Transform(Direction::Get(Direction3D::Forward), rotationMatrix) * size.z;

		Vector3 offsets[vertexNum] = {
			{-1, -1, -1}, {-1,  1, -1}, {1, -1, -1}, {1,  1, -1},
			{-1, -1,  1}, {-1,  1,  1}, {1, -1,  1}, {1,  1,  1}
		};

		for (int i = 0; i < vertexNum; ++i) {

			Vector3 localVertex = offsets[i].x * halfSizeX +
				offsets[i].y * halfSizeY +
				offsets[i].z * halfSizeZ;
			vertices[i] = centerPos + localVertex;
		}

		int edges[12][2] = {
			{0, 1}, {1, 3}, {3, 2}, {2, 0},
			{4, 5}, {5, 7}, {7, 6}, {6, 4},
			{0, 4}, {1, 5}, {2, 6}, {3, 7}
		};

		for (int i = 0; i < 12; ++i) {

			int start = edges[i][0];
			int end = edges[i][1];

			BaseLineRenderer::DrawLine(vertices[start], vertices[end], color, type);
		}
	}

	template<typename T>
	inline void LineRenderer3D::DrawCone(int division, float baseRadius, float topRadius, float height,
		const Vector3& centerPos, const T& rotation, const Color& color, LineType type) {

		const float kAngleStep = 2.0f * pi / division;

		std::vector<Vector3> baseCircle;
		std::vector<Vector3> topCircle;

		// 基底円と上面円の計算
		for (int i = 0; i <= division; ++i) {

			float angle = i * kAngleStep;
			baseCircle.emplace_back(baseRadius * std::cos(angle), 0.0f, baseRadius * std::sin(angle));
			topCircle.emplace_back(topRadius * std::cos(angle), height, topRadius * std::sin(angle));
		}

		Matrix4x4 rotationMatrix = Matrix4x4::MakeIdentity4x4();
		if constexpr (std::is_same_v<T, Vector3>) {

			rotationMatrix = Matrix4x4::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Quaternion>) {

			rotationMatrix = Quaternion::MakeRotateMatrix(rotation);
		} else if constexpr (std::is_same_v<T, Matrix4x4>) {

			rotationMatrix = rotation;
		}

		for (int i = 0; i < division; ++i) {

			// 円周上の点を回転＆平行移動
			Vector3 baseA = rotationMatrix.TransformPoint(baseCircle[i]) + centerPos;
			Vector3 baseB = rotationMatrix.TransformPoint(baseCircle[i + 1]) + centerPos;
			Vector3 topA = rotationMatrix.TransformPoint(topCircle[i]) + centerPos;
			Vector3 topB = rotationMatrix.TransformPoint(topCircle[i + 1]) + centerPos;

			// 円の描画
			BaseLineRenderer::DrawLine(baseA, baseB, color, type);
			BaseLineRenderer::DrawLine(topA, topB, color, type);

			// 側面の描画
			BaseLineRenderer::DrawLine(baseA, topA, color, type);
		}
	}
} // SakuEngine
#include "LineRenderer3D.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

// imgui
#include <imgui.h>

//============================================================================
//	LineRenderer3D classMethods
//============================================================================

void LineRenderer3D::ImGui() {

	ImGui::Checkbox("isDrawGrid", &isDrawGrid_);
	ImGui::DragInt("gridDivision", &gridDivision_, 1.0f, 1, 128);
	ImGui::DragFloat("gridSize", &gridSize_, 0.1f);
	ImGui::ColorEdit4("gridColor", &gridColor_.r);
}

void LineRenderer3D::DrawDebug() {

	if (isDrawGrid_) {

		DrawGrid(gridDivision_, gridSize_, gridColor_);
	}
}

void LineRenderer3D::DrawGrid(int32_t division, float gridSize, const Color& color, LineType type) {

	const float kGridEvery = (gridSize * 2.0f) / float(division);

	for (int index = 0; index <= division; ++index) {

		float offset = -gridSize + index * kGridEvery;

		// 横
		Vector3 verticalStart(offset, 0.0f, gridSize);
		Vector3 verticalEnd(offset, 0.0f, -gridSize);
		BaseLineRenderer::DrawLine(verticalStart, verticalEnd, color, type);

		// 縦
		Vector3 horizontalStart(-gridSize, 0.0f, offset);
		Vector3 horizontalEnd(gridSize, 0.0f, offset);
		BaseLineRenderer::DrawLine(horizontalStart, horizontalEnd, color, type);
	}
}

void LineRenderer3D::DrawSegment(int division, float radius, const Vector3& pointA,
	const Vector3& pointB, const Color& color, LineType type) {

	// 線分
	BaseLineRenderer::DrawLine(pointA, pointB, color, type);

	// AとBそれぞれに球
	DrawSphere(division, radius, pointA, color, type);
	DrawSphere(division, radius, pointB, color, type);
}

void LineRenderer3D::DrawSphere(int division, float radius, const Vector3& centerPos,
	const Color& color, LineType type) {

	const float kLatEvery = pi / division;        // 緯度
	const float kLonEvery = 2.0f * pi / division; // 経度

	auto calculatePoint = [&](float lat, float lon) -> Vector3 {
		return {
			radius * std::cos(lat) * std::cos(lon),
			radius * std::sin(lat),
			radius * std::cos(lat) * std::sin(lon)
		};
		};

	for (int latIndex = 0; latIndex < division; ++latIndex) {

		float lat = -pi / 2.0f + kLatEvery * latIndex;
		for (int lonIndex = 0; lonIndex < division; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			Vector3 pointA = calculatePoint(lat, lon);
			Vector3 pointB = calculatePoint(lat + kLatEvery, lon);
			Vector3 pointC = calculatePoint(lat, lon + kLonEvery);

			BaseLineRenderer::DrawLine(pointA + centerPos, pointB + centerPos, color, type);
			BaseLineRenderer::DrawLine(pointA + centerPos, pointC + centerPos, color, type);
		}
	}
}

void LineRenderer3D::DrawAABB(const Vector3& min, const Vector3& max,
	const Color& color, LineType type) {

	// AABBの各頂点
	std::vector<Vector3> vertices = {
		{min.x, min.y, min.z},
		{max.x, min.y, min.z},
		{min.x, max.y, min.z},
		{max.x, max.y, min.z},
		{min.x, min.y, max.z},
		{max.x, min.y, max.z},
		{min.x, max.y, max.z},
		{max.x, max.y, max.z} };

	// 各辺
	std::vector<std::pair<int, int>> edges = {
		{0, 1}, {1, 3}, {3, 2}, {2, 0}, // 前面
		{4, 5}, {5, 7}, {7, 6}, {6, 4}, // 背面
		{0, 4}, {1, 5}, {2, 6}, {3, 7}  // 前面と背面を繋ぐ辺
	};

	for (const auto& edge : edges) {

		const Vector3& start = vertices[edge.first];
		const Vector3& end = vertices[edge.second];

		// 各辺の描画
		BaseLineRenderer::DrawLine(start, end, color, type);
	}
}

void LineRenderer3D::DrawCircle(int division, float radius,
	const Vector3& center, const Color& color, LineType type) {

	for (int i = 0; i < division; ++i) {

		float a0 = 2.0f * pi * i / division;
		float a1 = 2.0f * pi * (i + 1) / division;

		Vector3 p0(center.x + radius * std::cos(a0), center.y, center.z + radius * std::sin(a0));
		Vector3 p1(center.x + radius * std::cos(a1), center.y, center.z + radius * std::sin(a1));

		BaseLineRenderer::DrawLine(p0, p1, color, type);
	}
}

void LineRenderer3D::DrawArc(int division, float radius, float halfAngle,
	const Vector3& center, const Vector3& direction, const Color& color, LineType type) {

	// XZ平面
	const float baseYaw = Math::GetYawRadian(direction);
	const float halfRadian = pi * halfAngle / 180.0f;

	// 始点と終点
	Vector3 firstPoint{};
	Vector3 lastPoint{};
	for (int i = 0; i < division; ++i) {

		float t0 = static_cast<float>(i) / division;
		float t1 = static_cast<float>(i + 1) / division;

		float a0 = baseYaw - halfRadian + (halfRadian * 2.0f) * t0;
		float a1 = baseYaw - halfRadian + (halfRadian * 2.0f) * t1;

		Vector3 p0(center.x + radius * std::cos(a0), center.y, center.z + radius * std::sin(a0));
		Vector3 p1(center.x + radius * std::cos(a1), center.y, center.z + radius * std::sin(a1));

		// 孤の描画
		BaseLineRenderer::DrawLine(p0, p1, color, type);

		// 始点と終点を保存
		if (i == 0) {
			firstPoint = p0;
		}
		if (i == division - 1) {
			lastPoint = p1;
		}
	}
	BaseLineRenderer::DrawLine(center, firstPoint, color, type);
	BaseLineRenderer::DrawLine(center, lastPoint, color, type);
}

void LineRenderer3D::DrawSquare(float length, const Vector3& center,
	const Color& color, LineType type) {

	float half = length * 0.5f;

	// 四隅の点
	Vector3 p0(center.x - half, center.y, center.z - half);
	Vector3 p1(center.x - half, center.y, center.z + half);
	Vector3 p2(center.x + half, center.y, center.z + half);
	Vector3 p3(center.x + half, center.y, center.z - half);

	// 線描画
	BaseLineRenderer::DrawLine(p0, p1, color, type);
	BaseLineRenderer::DrawLine(p1, p2, color, type);
	BaseLineRenderer::DrawLine(p2, p3, color, type);
	BaseLineRenderer::DrawLine(p3, p0, color, type);
}

void LineRenderer3D::DrawRect(const Vector2& size, const Vector3& center,
	const Color& color, LineType type) {

	float halfX = size.x * 0.5f;
	float halfY = size.y * 0.5f;

	// 四隅の点
	Vector3 p0(center.x - halfX, center.y, center.z - halfY);
	Vector3 p1(center.x - halfX, center.y, center.z + halfY);
	Vector3 p2(center.x + halfX, center.y, center.z + halfY);
	Vector3 p3(center.x + halfX, center.y, center.z - halfY);

	// 線描画
	BaseLineRenderer::DrawLine(p0, p1, color, type);
	BaseLineRenderer::DrawLine(p1, p2, color, type);
	BaseLineRenderer::DrawLine(p2, p3, color, type);
	BaseLineRenderer::DrawLine(p3, p0, color, type);
}
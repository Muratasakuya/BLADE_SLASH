#include "LineRenderer2D.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>

// imgui
#include <imgui.h>

//============================================================================
//	LineRenderer2D classMethods
//============================================================================

void LineRenderer2D::ImGui() {

	ImGui::Checkbox("isDrawGrid", &isDrawGrid_);
	ImGui::DragFloat2("gridSize", &gridSize_.x, 1.0f);
	ImGui::ColorEdit4("gridColor", &gridColor_.r);
}

void LineRenderer2D::DrawDebug() {

	// グリッド描画
	if (isDrawGrid_) {

		DrawFullScreenGrid(gridSize_, gridColor_, LineType::DepthIgnore);
	}
}

void LineRenderer2D::DrawFullScreenGrid(const Vector2& gridSize, const Color& color, LineType type) {

	// グリッドサイズから縦横何分割するか計算
	uint32_t horizontalCount = static_cast<uint32_t>(Config::kWindowWidth / gridSize.x);
	uint32_t verticalCount = static_cast<uint32_t>(Config::kWindowHeight / gridSize.y);

	// 横線描画
	for (uint32_t i = 0; i < verticalCount + 1; ++i) {

		float y = gridSize.y * static_cast<float>(i);
		Vector2 pointA = Vector2(0.0f, y);
		Vector2 pointB = Vector2(static_cast<float>(Config::kWindowWidth), y);
		BaseLineRenderer::DrawLine(pointA, pointB, color, type);
	}
	// 縦線描画
	for (uint32_t i = 0; i < horizontalCount + 1; ++i) {

		float x = gridSize.x * static_cast<float>(i);
		Vector2 pointA = Vector2(x, 0.0f);
		Vector2 pointB = Vector2(x, static_cast<float>(Config::kWindowHeight));
		BaseLineRenderer::DrawLine(pointA, pointB, color, type);
	}
}

void LineRenderer2D::DrawRect(const Vector2& center, const Vector2& size,
	const Vector2& anchor, const Color& color, LineType type) {

	// サイズ
	Vector2 rectSize = size * anchor;

	// 4頂点座標計算
	Vector2 topLeft = Vector2(center.x - rectSize.x, center.y - rectSize.y);
	Vector2 topRight = Vector2(center.x + rectSize.x, center.y - rectSize.y);
	Vector2 bottomLeft = Vector2(center.x - rectSize.x, center.y + rectSize.y);
	Vector2 bottomRight = Vector2(center.x + rectSize.x, center.y + rectSize.y);

	// 4辺のライン描画
	BaseLineRenderer::DrawLine(topLeft, topRight, color, type);
	BaseLineRenderer::DrawLine(topRight, bottomRight, color, type);
	BaseLineRenderer::DrawLine(bottomRight, bottomLeft, color, type);
	BaseLineRenderer::DrawLine(bottomLeft, topLeft, color, type);
}
#include "LineRenderer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Scene/SceneView.h>

//============================================================================
//	LineRenderer classMethods
//============================================================================

LineRenderer* LineRenderer::instance_ = nullptr;

LineRenderer* LineRenderer::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new LineRenderer();
	}
	return instance_;
}

void LineRenderer::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void LineRenderer::Init(ID3D12Device8* device, ID3D12GraphicsCommandList* commandList,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler, SceneView* sceneView) {

	commandList_ = nullptr;
	commandList_ = commandList;

	sceneView_ = nullptr;
	sceneView_ = sceneView;

	// 各描画情報を初期化
	renderData_[LineType::None].Init("PrimitiveLine.json", device, srvDescriptor, shaderCompiler);
	renderData_[LineType::DepthIgnore].Init("DepthIgnorePrimitiveLine.json", device, srvDescriptor, shaderCompiler);

	viewProjectionBuffer_.CreateBuffer(device);
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	debugSceneViewProjectionBuffer_.CreateBuffer(device);
#endif
}

void LineRenderer::RenderStructure::Init(const std::string& pipelineFile, ID3D12Device8* device,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler) {

	pipeline = std::make_unique<PipelineState>();
	pipeline->Create(pipelineFile, device, srvDescriptor, shaderCompiler);

	vertexBuffer.CreateBuffer(device, kMaxLineCount_ * kVertexCountLine_);
}

void LineRenderer::DrawLine3D(const Vector3& pointA, const Vector3& pointB,
	const Color& color, LineType type) {

	auto& lineVertices = renderData_[type].lineVertices;
	ASSERT(lineVertices.size() < kMaxLineCount_ * kVertexCountLine_, "exceeded the upper limit line");

	// 頂点追加
	lineVertices.emplace_back(Vector4(pointA.x, pointA.y, pointA.z, 1.0f), color);
	lineVertices.emplace_back(Vector4(pointB.x, pointB.y, pointB.z, 1.0f), color);
}

void LineRenderer::ExecuteLine(bool debugEnable, LineType type) {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	auto& renderData = renderData_[type];
	if (renderData.lineVertices.empty()) {
		return;
	}

	commandList_->SetGraphicsRootSignature(renderData.pipeline->GetRootSignature());
	commandList_->SetPipelineState(renderData.pipeline->GetGraphicsPipeline());
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	renderData.vertexBuffer.TransferData(renderData.lineVertices);
	commandList_->IASetVertexBuffers(0, 1, &renderData.vertexBuffer.GetVertexBufferView());

	if (!debugEnable) {

		viewProjectionBuffer_.TransferData(sceneView_->GetCamera()->GetViewProjectionMatrix());
		commandList_->SetGraphicsRootConstantBufferView(0, viewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());
	} else {

		debugSceneViewProjectionBuffer_.TransferData(sceneView_->GetSceneCamera()->GetViewProjectionMatrix());
		commandList_->SetGraphicsRootConstantBufferView(0, debugSceneViewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());
	}

	commandList_->DrawInstanced(static_cast<UINT>(renderData.lineVertices.size()), 1, 0, 0);
#endif
}

void LineRenderer::ResetLine() {

	renderData_[LineType::None].lineVertices.clear();
	renderData_[LineType::DepthIgnore].lineVertices.clear();
}

void LineRenderer::DrawGrid(int division, float gridSize, const Color& color, LineType type) {

	const float kGridEvery = (gridSize * 2.0f) / float(division);

	for (int index = 0; index <= division; ++index) {

		float offset = -gridSize + index * kGridEvery;

		// 横
		Vector3 verticalStart(offset, 0.0f, gridSize);
		Vector3 verticalEnd(offset, 0.0f, -gridSize);
		DrawLine3D(verticalStart, verticalEnd, color, type);

		// 縦
		Vector3 horizontalStart(-gridSize, 0.0f, offset);
		Vector3 horizontalEnd(gridSize, 0.0f, offset);
		DrawLine3D(horizontalStart, horizontalEnd, color, type);
	}
}

void LineRenderer::DrawSegment(int division, float radius, const Vector3& pointA,
	const Vector3& pointB, const Color& color, LineType type) {

	// 線分
	DrawLine3D(pointA, pointB, color, type);

	// AとBそれぞれに球
	DrawSphere(division, radius, pointA, color, type);
	DrawSphere(division, radius, pointB, color, type);
}

void LineRenderer::DrawSphere(int division, float radius, const Vector3& centerPos,
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

			DrawLine3D(pointA + centerPos, pointB + centerPos, color, type);
			DrawLine3D(pointA + centerPos, pointC + centerPos, color, type);
		}
	}
}

void LineRenderer::DrawAABB(const Vector3& min, const Vector3& max,
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
		DrawLine3D(start, end, color, type);
	}
}

void LineRenderer::DrawCircle(int division, float radius,
	const Vector3& center, const Color& color, LineType type) {

	for (int i = 0; i < division; ++i) {

		float a0 = 2.0f * pi * i / division;
		float a1 = 2.0f * pi * (i + 1) / division;

		Vector3 p0(center.x + radius * std::cos(a0), center.y, center.z + radius * std::sin(a0));
		Vector3 p1(center.x + radius * std::cos(a1), center.y, center.z + radius * std::sin(a1));

		DrawLine3D(p0, p1, color, type);
	}
}

void LineRenderer::DrawArc(int division, float radius, float halfAngle,
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
		DrawLine3D(p0, p1, color);

		// 始点と終点を保存
		if (i == 0) {
			firstPoint = p0;
		}
		if (i == division - 1) {
			lastPoint = p1;
		}
	}
	DrawLine3D(center, firstPoint, color, type);
	DrawLine3D(center, lastPoint, color, type);
}

void LineRenderer::DrawSquare(float length, const Vector3& center,
	const Color& color, LineType type) {

	float half = length * 0.5f;

	// 四隅の点
	Vector3 p0(center.x - half, center.y, center.z - half);
	Vector3 p1(center.x - half, center.y, center.z + half);
	Vector3 p2(center.x + half, center.y, center.z + half);
	Vector3 p3(center.x + half, center.y, center.z - half);

	// 線描画
	DrawLine3D(p0, p1, color, type);
	DrawLine3D(p1, p2, color, type);
	DrawLine3D(p2, p3, color, type);
	DrawLine3D(p3, p0, color, type);
}

void LineRenderer::DrawRect(const Vector2& size, const Vector3& center,
	const Color& color, LineType type) {

	float halfX = size.x * 0.5f;
	float halfY = size.y * 0.5f;

	// 四隅の点
	Vector3 p0(center.x - halfX, center.y, center.z - halfY);
	Vector3 p1(center.x - halfX, center.y, center.z + halfY);
	Vector3 p2(center.x + halfX, center.y, center.z + halfY);
	Vector3 p3(center.x + halfX, center.y, center.z - halfY);

	// 線描画
	DrawLine3D(p0, p1, color, type);
	DrawLine3D(p1, p2, color, type);
	DrawLine3D(p2, p3, color, type);
	DrawLine3D(p3, p0, color, type);
}

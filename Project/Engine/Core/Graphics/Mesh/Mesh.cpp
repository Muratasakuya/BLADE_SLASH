#include "Mesh.h"

//============================================================================
//	IMesh classMethods
//============================================================================

void IMesh::Init(ID3D12Device* device, const ResourceMesh<MeshVertex>& resource,
	bool isSkinned, uint32_t numInstance) {

	isSkinned_ = isSkinned;

	// meshの数分
	for (uint32_t meshIndex = 0; meshIndex < resource.meshCount_; ++meshIndex) {

		// 頂点数
		vertexCounts_.push_back(static_cast<UINT>(resource.vertices[meshIndex].size()));
		indexCounts_.push_back(static_cast<UINT>(resource.indices[meshIndex].size()));
		// meshlet数
		meshletCounts_.push_back(static_cast<uint32_t>(resource.meshlets[meshIndex].size()));

		// buffer生成
		CreateBuffer(device, meshIndex, resource);
		CreateVertexBuffer(device, meshIndex, resource, numInstance);

		// buffer転送
		TransferBuffer(meshIndex, resource, isSkinned);
		TransferVertexBuffer(meshIndex, resource);
	}
}

void IMesh::CreateBuffer(ID3D12Device* device, uint32_t meshIndex,
	const ResourceMesh<MeshVertex>& resource) {

	// meshInstance情報
	meshInstanceData_.push_back({});
	meshInstanceData_[meshIndex].CreateBuffer(device);
	// インデックス
	const UINT uniqueVertexIndexCount = static_cast<UINT>(resource.uniqueVertexIndices[meshIndex].size());
	uniqueVertexIndices_.push_back({});
	uniqueVertexIndices_[meshIndex].CreateSRVBuffer(device, uniqueVertexIndexCount);
	// プリミティブ
	const UINT primitiveIndexCount = static_cast<UINT>(resource.primitiveIndices[meshIndex].size());
	primitiveIndices_.push_back({});
	primitiveIndices_[meshIndex].CreateSRVBuffer(device, primitiveIndexCount);
	// meshlet
	meshlets_.push_back({});
	meshlets_[meshIndex].CreateSRVBuffer(device, meshletCounts_[meshIndex]);

	indices_.push_back({});
	indices_[meshIndex].CreateBuffer(device, indexCounts_[meshIndex]);
}

void IMesh::TransferBuffer(uint32_t meshIndex, const ResourceMesh<MeshVertex>& resource, bool isSkinned) {

	// meshInstance情報
	meshInstanceData_[meshIndex].TransferData({
		.meshletCount = meshletCounts_[meshIndex],
		.numVertices = vertexCounts_[meshIndex],
		.isSkinned = static_cast<int32_t>(isSkinned) });
	// インデックス
	uniqueVertexIndices_[meshIndex].TransferData(resource.uniqueVertexIndices[meshIndex]);
	// プリミティブ
	primitiveIndices_[meshIndex].TransferData(resource.primitiveIndices[meshIndex]);
	// meshlet
	meshlets_[meshIndex].TransferData(resource.meshlets[meshIndex]);

	indices_[meshIndex].TransferData(resource.indices[meshIndex]);
}

//============================================================================
//	StaticMesh classMethods
//============================================================================

void StaticMesh::CreateVertexBuffer(ID3D12Device* device, uint32_t meshIndex,
	[[maybe_unused]] const ResourceMesh<MeshVertex>& resource, [[maybe_unused]] uint32_t numInstance) {

	// 頂点
	vertices_.push_back({});
	vertices_[meshIndex].CreateSRVBuffer(device, vertexCounts_[meshIndex]);
}

void StaticMesh::TransferVertexBuffer(uint32_t meshIndex, const ResourceMesh<MeshVertex>& resource) {

	// 頂点
	vertices_[meshIndex].TransferData(resource.vertices[meshIndex]);
}

//============================================================================
//	SkinnedMesh classMethods
//============================================================================

void SkinnedMesh::CreateVertexBuffer(ID3D12Device* device, uint32_t meshIndex,
	[[maybe_unused]] const ResourceMesh<MeshVertex>& resource, uint32_t numInstance) {

	// 入力頂点
	inputVertices_.push_back({});
	inputVertices_[meshIndex].CreateSRVBuffer(device, vertexCounts_[meshIndex]);
	// 出力頂点
	outputVertices_.push_back({});
	outputVertices_[meshIndex].CreateUAVBuffer(device, vertexCounts_[meshIndex] * numInstance);
}

void SkinnedMesh::TransferVertexBuffer(uint32_t meshIndex, const ResourceMesh<MeshVertex>& resource) {

	// 入力頂点
	inputVertices_[meshIndex].TransferData(resource.vertices[meshIndex]);
}

//============================================================================
//	EffectMesh classMethods
//============================================================================

void EffectMesh::Init(ID3D12Device* device,
	const ResourceMesh<EffectMeshVertex>& resource, uint32_t numInstance) {

	// 頂点数
	vertexCount_ = static_cast<UINT>(resource.vertices.front().size());
	// meshlet数
	meshletCount_ = static_cast<uint32_t>(resource.meshlets.front().size());

	// buffer生成
	CreateBuffer(device, resource, numInstance);
	// buffer転送
	TransferBuffer(resource);
}

void EffectMesh::CreateBuffer(ID3D12Device* device,
	const ResourceMesh<EffectMeshVertex>& resource, [[maybe_unused]] uint32_t numInstance) {

	// meshInstance情報
	meshInstanceData_.CreateBuffer(device);
	// 頂点
	vertices_.CreateSRVBuffer(device, vertexCount_);
	// インデックス
	const UINT uniqueVertexIndexCount = static_cast<UINT>(resource.uniqueVertexIndices.front().size());
	uniqueVertexIndices_.CreateSRVBuffer(device, uniqueVertexIndexCount);
	// プリミティブ
	const UINT primitiveIndexCount = static_cast<UINT>(resource.primitiveIndices.front().size());
	primitiveIndices_.CreateSRVBuffer(device, primitiveIndexCount);
	// meshlet
	meshlets_.CreateSRVBuffer(device, meshletCount_);
}

void EffectMesh::TransferBuffer(const ResourceMesh<EffectMeshVertex>& resource) {

	// meshInstance情報
	meshInstanceData_.TransferData({
		.meshletCount = meshletCount_,
		.numVertices = vertexCount_ });
	// 頂点
	vertices_.TransferData(resource.vertices.front());
	// インデックス
	uniqueVertexIndices_.TransferData(resource.uniqueVertexIndices.front());
	// プリミティブ
	primitiveIndices_.TransferData(resource.primitiveIndices.front());
	// meshlet
	meshlets_.TransferData(resource.meshlets.front());
}
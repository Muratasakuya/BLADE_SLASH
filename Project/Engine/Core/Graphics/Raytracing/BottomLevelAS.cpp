#include "BottomLevelAS.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Mesh/Mesh.h>

//============================================================================
//	BottomLevelAS classMethods
//============================================================================

static void FillGeometryDesc(D3D12_RAYTRACING_GEOMETRY_DESC& geometryDesc, IMesh* mesh, uint32_t meshIndex) {

	const bool skinned = mesh->IsSkinned();

	// 頂点buffer取得
	// skinnedかどうかで分岐させる
	const DxStructuredBuffer<MeshVertex>& vertexBuffer = skinned
		? static_cast<SkinnedMesh*>(mesh)->GetOutputVertexBuffer(meshIndex)
		: static_cast<StaticMesh*> (mesh)->GetVertexBuffer(meshIndex);
	const IndexBuffer& indexBuffer = mesh->GetIndexBuffer(meshIndex);

	// descの設定
	geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer.GetResource()->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(MeshVertex);
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // 頂点(pos)
	geometryDesc.Triangles.VertexCount = mesh->GetVertexCount(meshIndex);

	geometryDesc.Triangles.IndexBuffer = indexBuffer.GetResource()->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = mesh->GetIndexCount(meshIndex);
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
}

void BottomLevelAS::Build(ID3D12Device8* device,
	ID3D12GraphicsCommandList6* commandList, const BuildRequest& request) {

	allowUpdate_ = request.allowUpdate;

	FillGeometryDesc(geometryDesc_, request.mesh, request.meshIndex);

	// Inputs設定
	inputs_.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs_.Flags = allowUpdate_ ?
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE :
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs_.NumDescs = 1;
	inputs_.pGeometryDescs = &geometryDesc_;

	// バッファサイズ取得
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO buildInfo{};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs_, &buildInfo);

	// リソース作成処理
	scratch_.Create(device, buildInfo.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON);
	result_.Create(device, buildInfo.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

	// buildDesc設定
	buildDesc_.DestAccelerationStructureData = result_.GetResource()->GetGPUVirtualAddress();
	buildDesc_.ScratchAccelerationStructureData = scratch_.GetResource()->GetGPUVirtualAddress();
	buildDesc_.SourceAccelerationStructureData = 0;
	buildDesc_.Inputs = inputs_;

	// build実行
	commandList->BuildRaytracingAccelerationStructure(&buildDesc_, 0, nullptr);

	// UAVバリア
	D3D12_RESOURCE_BARRIER uavBarrier{};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = result_.GetResource();
	commandList->ResourceBarrier(1, &uavBarrier);
}

void BottomLevelAS::Update(ID3D12GraphicsCommandList6* commandList) {

	// skinnedMeshじゃない場合は更新しない
	if (!allowUpdate_) {
		return;
	}

	// 前フレームのresultをsourceとして指定
	buildDesc_.SourceAccelerationStructureData = result_.GetResource()->GetGPUVirtualAddress();
	buildDesc_.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	commandList->BuildRaytracingAccelerationStructure(&buildDesc_, 0, nullptr);

	// バリア遷移
	D3D12_RESOURCE_BARRIER uavBarrier{};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = result_.GetResource();
	commandList->ResourceBarrier(1, &uavBarrier);
}
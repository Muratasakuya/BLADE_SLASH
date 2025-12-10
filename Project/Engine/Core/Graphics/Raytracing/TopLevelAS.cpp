#include "TopLevelAS.h"

using namespace SakuEngine;

//============================================================================
//	TopLevelAS classMethods
//============================================================================

void TopLevelAS::Build(ID3D12Device8* device, ID3D12GraphicsCommandList6* commandList,
	const std::vector<RayTracingInstance>& instances, bool allowUpdate) {

	device_ = nullptr;
	device_ = device;

	allowUpdate_ = allowUpdate;

	// instanceDescバッファ生成
	const UINT64 descBytes = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances.size();
	instanceDescs_.Create(device, descBytes, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);

	// マッピング処理
	D3D12_RAYTRACING_INSTANCE_DESC* mapped = nullptr;
	instanceDescs_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mapped));

	for (size_t i = 0; i < instances.size(); ++i) {

		const RayTracingInstance& instance = instances[i];
		auto& data = mapped[i];

		CopyMatrix3x4(data.Transform, Matrix4x4::Transpose(instance.matrix));
		data.InstanceID = instance.instanceID;
		data.InstanceContributionToHitGroupIndex = instance.hitGroupIdx;
		data.InstanceMask = instance.mask;
		data.Flags = instance.flags;
		data.AccelerationStructure = instance.blas->GetGPUVirtualAddress();
	}
	instanceDescs_.GetResource()->Unmap(0, nullptr);

	// Inputs
	inputs_ = {};
	inputs_.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs_.Flags = allowUpdate_ ?
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE :
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs_.NumDescs = static_cast<UINT>(instances.size());
	inputs_.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs_.InstanceDescs = instanceDescs_.GetResource()->GetGPUVirtualAddress();

	// サイズ問い合わせ
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs_, &info);

	scratch_.Create(device, info.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON);
	result_.Create(device, info.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

	// buildDesc設定
	buildDesc_.DestAccelerationStructureData = result_.GetResource()->GetGPUVirtualAddress();
	buildDesc_.ScratchAccelerationStructureData = scratch_.GetResource()->GetGPUVirtualAddress();
	buildDesc_.SourceAccelerationStructureData = 0;
	buildDesc_.Inputs = inputs_;

	// build実行
	commandList->BuildRaytracingAccelerationStructure(&buildDesc_, 0, nullptr);

	// バリア遷移
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = result_.GetResource();
	commandList->ResourceBarrier(1, &barrier);
}

void TopLevelAS::Update(ID3D12GraphicsCommandList6* commandList, const std::vector<RayTracingInstance>& instances) {

	// skinnedMeshじゃない場合は更新しない
	if (!allowUpdate_) {
		return;
	}

	// インスタンスが変わったら再ビルドし直す
	if (instances.size() != inputs_.NumDescs) {

		Build(device_, commandList, instances, allowUpdate_);
		return;
	}

	D3D12_RAYTRACING_INSTANCE_DESC* mapped = nullptr;
	instanceDescs_.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mapped));

	for (size_t i = 0; i < instances.size(); ++i) {

		const RayTracingInstance& instance = instances[i];
		auto& data = mapped[i];

		CopyMatrix3x4(data.Transform, Matrix4x4::Transpose(instance.matrix));
		data.InstanceID = instance.instanceID;
		data.InstanceContributionToHitGroupIndex = instance.hitGroupIdx;
		data.InstanceMask = instance.mask;
		data.Flags = instance.flags;
		data.AccelerationStructure = instance.blas->GetGPUVirtualAddress();
	}
	instanceDescs_.GetResource()->Unmap(0, nullptr);

	// PERFORM_UPDATEをセット
	buildDesc_.SourceAccelerationStructureData = result_.GetResource()->GetGPUVirtualAddress();
	buildDesc_.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	commandList->BuildRaytracingAccelerationStructure(&buildDesc_, 0, nullptr);

	// バリア遷移
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = result_.GetResource();
	commandList->ResourceBarrier(1, &barrier);
}

void TopLevelAS::CopyMatrix3x4(float(&dst)[3][4], const Matrix4x4& src) {

	for (int r = 0; r < 3; ++r) {
		for (int c = 0; c < 4; ++c) {

			dst[r][c] = src.m[r][c];
		}
	}
}

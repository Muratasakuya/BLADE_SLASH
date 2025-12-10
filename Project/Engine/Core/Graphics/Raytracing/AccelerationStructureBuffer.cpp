#include "AccelerationStructureBuffer.h"

using namespace SakuEngine;

//============================================================================
//	AccelerationStructureBuffer classMethods
//============================================================================

void AccelerationStructureBuffer::Create(ID3D12Device* device, UINT64 sizeInBytes,
	D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, D3D12_HEAP_TYPE heapType) {

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = heapType;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = sizeInBytes;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = flags;

	HRESULT hr = device->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE, &desc,
		initState, nullptr, IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr));
}

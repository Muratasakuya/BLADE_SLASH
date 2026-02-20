#include "DxRootSignature.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>

//============================================================================
//	DxRootSignature classMethods
//============================================================================

void DxRootSignature::Create(const std::string& fileName, const Json& json,
	ID3D12Device* device, SRVDescriptor* srvDescriptor, ComPtr<ID3D12RootSignature>& rootSignature) {

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

	bool isMeshShader = false;

	// meshShaderを使うかどうか
	if (json.contains("Type") && json["Type"] == "MS") {

		// typeがMS
		isMeshShader = true;
	}

	// フラグ設定
	if (isMeshShader) {

		auto flag = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		rootSignatureDesc.Flags = flag;
	} else {

		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	}

	// Jsonから受け取ったDataで設定
	{
		static std::vector<D3D12_ROOT_PARAMETER> rootParameters;
		static std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptorRanges;
		static std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

		rootParameters.clear();
		descriptorRanges.clear();
		staticSamplers.clear();

		// Root Parameters の解析
		for (const auto& param : json["RootParameter"]) {

			D3D12_ROOT_PARAMETER rootParam{};
			std::string type = param["Type"];
			std::string visibility = param["Visibility"];

			if (type == "CBV") {

				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				rootParam.Descriptor.ShaderRegister = param["ShaderRegister"];
			}
			if (type == "UAV") {

				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
				rootParam.Descriptor.ShaderRegister = param["ShaderRegister"];
			}
			if (type == "SRV") {

				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
				rootParam.Descriptor.ShaderRegister = param["ShaderRegister"];
			} else if (type == "TABLE") {

				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

				// DescriptorRange のメモリを確保
				descriptorRanges.emplace_back();   // 新しい vector を追加
				descriptorRanges.back().resize(1); // 1つの DescriptorRangeを確保

				D3D12_DESCRIPTOR_RANGE& descRange = descriptorRanges.back()[0]; // 参照を取得

				descRange.BaseShaderRegister = param["DescriptorRange"]["BaseShaderRegister"];

				UINT numDescriptors = 1;
				if (param["DescriptorRange"].contains("NumDescriptors") &&
					param["DescriptorRange"]["NumDescriptors"] == "srvCount") {

					// srvの数だけ設定する
					numDescriptors = srvDescriptor->GetMaxSRVCount();
				}

				if (param["DescriptorRange"].contains("RegisterSpace")) {

					// registerSpaceを設定
					descRange.RegisterSpace = param["DescriptorRange"]["RegisterSpace"];
				}

				descRange.NumDescriptors = numDescriptors;
				descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				std::string descRangeType = param["DescriptorRange"]["Type"];
				if (descRangeType == "SRV") {

					descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				} else if (descRangeType == "UAV") {

					descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				}

				rootParam.DescriptorTable.pDescriptorRanges = descriptorRanges.back().data();
				rootParam.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptorRanges.back().size());
			}

			// Visibility の設定
			if (visibility == "VERTEX") {

				rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			} else if (visibility == "MESH") {

				rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;
			} else if (visibility == "PIXEL") {

				rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			} else if (visibility == "COMPUTE") {

				rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}

			rootParameters.emplace_back(rootParam);
		}

		if (json.contains("StaticSampler")) {
			// Static Samplers の解析
			for (const auto& sampler : json["StaticSampler"]) {

				D3D12_STATIC_SAMPLER_DESC staticSampler{};
				std::string filterType = sampler["Filter"];
				std::string addressMode = sampler["AddressMode"];
				std::string comparisonFunc = sampler["ComparisonFunc"];

				// フィルター設定
				if (filterType == "LINEAR") {

					staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				} else if (filterType == "COMPARISON_LINEAR") {

					staticSampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
				} else if (filterType == "POINT") {

					staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				}

				// アドレスモード設定
				if (addressMode == "WRAP") {

					staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
					staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
					staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				} else if (addressMode == "MIRROR") {

					staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
					staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
					staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
				} else if (addressMode == "CLAMP") {

					staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
					staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
					staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				} else if (addressMode == "BORDER") {

					staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
					staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
					staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
				} else if (addressMode == "MIRROR_ONCE") {

					staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
					staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
					staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
				}

				if (comparisonFunc == "NEVER") {

					staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				} else if (comparisonFunc == "GREATER") {

					staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER;
				}

				// Visibilityの設定
				staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				staticSampler.ShaderRegister = sampler["ShaderRegister"];
				staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

				staticSamplers.emplace_back(staticSampler);
			}

			rootSignatureDesc.pStaticSamplers = staticSamplers.data();
			rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers.size());
		}

		// D3D12_ROOT_SIGNATURE_DESC に設定
		rootSignatureDesc.pParameters = rootParameters.data();
		rootSignatureDesc.NumParameters = static_cast<UINT>(rootParameters.size());
	}

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	// バイナリをもとに生成
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {

		const char* errorMessage = reinterpret_cast<const char*>(errorBlob->GetBufferPointer());

		const std::string& file = "FileName: " + fileName + "\n";
		ASSERT(false, file + "Failed to create rootSignature: " + std::string(errorMessage));
	}
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

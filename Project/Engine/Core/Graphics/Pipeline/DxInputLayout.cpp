#include "DxInputLayout.h"

//============================================================================
//	DxInputLayout classMethods
//============================================================================

void DxInputLayout::Create(const Json& json, std::optional<D3D12_INPUT_LAYOUT_DESC>& desc) {

	if (!json.contains("InputLayout") || !json["InputLayout"].is_array()) {
		desc = std::nullopt;
		return;
	}

	static std::vector<D3D12_INPUT_ELEMENT_DESC> elements{};
	static std::vector<std::unique_ptr<std::string>> semanticNames{};
	static D3D12_INPUT_ELEMENT_DESC element{};

	elements.clear();
	semanticNames.clear();
	for (const auto& input : json["InputLayout"]) {
		if (!input.contains("SemanticName") || !input.contains("Format")) {
			continue;
		}

		element = D3D12_INPUT_ELEMENT_DESC();

		element.SemanticIndex = 0;
		element.InputSlot = 0;
		element.InstanceDataStepRate = 0;
		element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

		semanticNames.emplace_back(std::make_unique<std::string>(input["SemanticName"].get<std::string>()));
		element.SemanticName = semanticNames.back()->c_str();
		std::string formatStr = input["Format"].get<std::string>();
		if (formatStr == "R32G32B32A32_FLOAT") {
			element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		} else if (formatStr == "R32G32B32_FLOAT") {
			element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		} else if (formatStr == "R32G32_FLOAT") {
			element.Format = DXGI_FORMAT_R32G32_FLOAT;
		} else {
			continue;
		}

		elements.push_back(element);
	}

	if (elements.empty()) {
		desc = std::nullopt;
		return;
	}

	// D3D12_INPUT_LAYOUT_DESCの設定
	D3D12_INPUT_LAYOUT_DESC layoutDesc = {};
	layoutDesc.pInputElementDescs = elements.data();
	layoutDesc.NumElements = static_cast<UINT>(elements.size());

	desc = layoutDesc;
}
#include "DxDepthRaster.h"

using namespace SakuEngine;

//============================================================================
//	DxDepthRaster classMethods
//============================================================================

void DxDepthRaster::Create(const Json& json,
	D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_DEPTH_STENCIL_DESC& depthStencilDesc) {

	//============================================================================
	//	RasterizerDesc

	if (json.contains("Rasterizer") && json["Rasterizer"].is_array() && !json["Rasterizer"].empty()) {

		const auto& raster = json["Rasterizer"][0];

		// CullMode の設定
		std::string cullModeStr = raster.value("CullMode", "BACK");
		if (cullModeStr == "NONE") {
			rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		} else if (cullModeStr == "FRONT") {
			rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
		} else if (cullModeStr == "BACK") {
			rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		}

		std::string fillModeStr = raster.value("FillMode", "SOLID");
		rasterizerDesc.FillMode = (fillModeStr == "WIREFRAME") ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
		rasterizerDesc.DepthClipEnable = (raster.value("DepthClipEnable", "TRUE") == "TRUE") ? TRUE : FALSE;
		rasterizerDesc.AntialiasedLineEnable = (raster.value("AntialiasedLineEnable", "FALSE") == "TRUE") ? TRUE : FALSE;
	}

	//============================================================================
	//	DepthStencilDesc

	if (json.contains("DepthStencil") && json["DepthStencil"].is_array() && !json["DepthStencil"].empty()) {

		const auto& depthStencil = json["DepthStencil"][0];

		// DepthFunc の設定
		std::string depthFuncStr = depthStencil.value("DepthFunc", "LESS_EQUAL");
		if (depthFuncStr == "NEVER") {
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
		} else if (depthFuncStr == "LESS") {
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		} else if (depthFuncStr == "EQUAL") {
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
		} else if (depthFuncStr == "LESS_EQUAL") {
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		} else if (depthFuncStr == "GREATER") {
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
		} else if (depthFuncStr == "NOT_EQUAL") {
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		} else if (depthFuncStr == "GREATER_EQUAL") {
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		} else if (depthFuncStr == "ALWAYS") {
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		}

		depthStencilDesc.DepthEnable = (depthStencil.value("DepthEnable", "TRUE") == "TRUE") ? TRUE : FALSE;
		std::string depthWriteMaskStr = depthStencil.value("DepthWriteMask", "ALL");
		depthStencilDesc.DepthWriteMask = (depthWriteMaskStr == "ZERO") ? D3D12_DEPTH_WRITE_MASK_ZERO : D3D12_DEPTH_WRITE_MASK_ALL;
	}
}

#include "MeshRender.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	MeshRender classMethods
//============================================================================

void MeshRender::Init(const std::string& name) {

	// 名前を設定
	modelName = name;

	// デフォルト
	renderView = MeshRenderView::Both;
	blendMode = BlendMode::kBlendModeNormal;
}

void MeshRender::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	ImGui::Text("modelName: %s", modelName.c_str());
	SakuEngine::EnumAdapter<MeshRenderView>::Combo("RenderView", &renderView);
	SakuEngine::EnumAdapter<BlendMode>::Combo("BlendMode", &blendMode);

	ImGui::PopItemWidth();
}

void MeshRender::FromJson(const Json& data) {

	renderView = SakuEngine::EnumAdapter<MeshRenderView>::FromString(
		data.value("renderView", "Both")).value();
	blendMode = SakuEngine::EnumAdapter<BlendMode>::FromString(
		data.value("blendMode", "kBlendModeNormal")).value();
}

void MeshRender::ToJson(Json& data) {

	data["renderView"] = SakuEngine::EnumAdapter<MeshRenderView>::ToString(renderView);
	data["blendMode"] = SakuEngine::EnumAdapter<BlendMode>::ToString(blendMode);
}

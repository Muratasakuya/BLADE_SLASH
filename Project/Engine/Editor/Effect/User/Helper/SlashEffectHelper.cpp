#include "SlashEffectHelper.h"

using namespace SakuEngine;

//============================================================================
//	SlashEffectHelper classMethods
//============================================================================

void SlashEffectHelper::SetParent(const std::string& nodeName, const Transform3D& transform) {

	// 親の設定
	effectNodeName = nodeName;
	effect->SetParent(nodeName, transform);
}

void SlashEffectHelper::Emit(const GameObject3D& object) {

	// エフェクト発生
	effect->Emit(object.GetRotation() * effectOffset);
}

void SlashEffectHelper::Update(const GameObject3D& object) {

	// 剣エフェクトの更新、親の回転を設定する
	effect->SetParentRotation(effectNodeName, Quaternion::Normalize(object.GetRotation()),
		ParticleUpdateModuleID::Rotation);
	effect->Update();
}

void SlashEffectHelper::EditOffset(const std::string& label) {

	ImGui::DragFloat3(label.c_str(), &effectOffset.x, 0.1f);
}

void SlashEffectHelper::FromJson(const Json& data, const std::string& label) {

	effectOffset = Vector3::FromJson(data.value(label, Json()));
}

void SlashEffectHelper::ToJson(Json& data, const std::string& label) const {

	data[label] = effectOffset.ToJson();
}
#include "ParticleUpdateSpeedAdjustmentModule.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateSpeedAdjustmentModule classMethods
//============================================================================

void ParticleUpdateSpeedAdjustmentModule::Init() {

	// 初期化値
	calSign_ = Sign::Plus;
	adjustmentValue_ = 1.0f;
}

void ParticleUpdateSpeedAdjustmentModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// 符号に合わせて計算
	switch (calSign_) {
	case Sign::Plus:

		particle.velocity += adjustmentValue_;
		break;
	case Sign::Minus:

		particle.velocity -= adjustmentValue_;
		break;
	}
}

void ParticleUpdateSpeedAdjustmentModule::ImGui() {

	EnumAdapter<Sign>::Combo("Sign", &calSign_);
	ImGui::DragFloat("adjustmentValue", &adjustmentValue_, 0.01f);
}

Json ParticleUpdateSpeedAdjustmentModule::ToJson() {

	Json data;

	data["calSign_"] = EnumAdapter<Sign>::ToString(calSign_);
	data["adjustmentValue_"] = adjustmentValue_;

	return data;
}

void ParticleUpdateSpeedAdjustmentModule::FromJson(const Json& data) {

	calSign_ = EnumAdapter<Sign>::FromString(data.value("calSign_", "Plus")).value();
	adjustmentValue_ = data.value("adjustmentValue_", 1.0f);
}
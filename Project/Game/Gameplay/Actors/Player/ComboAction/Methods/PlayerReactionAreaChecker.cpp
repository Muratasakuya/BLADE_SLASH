#include "PlayerReactionAreaChecker.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerReactionAreaChecker classMethods
//============================================================================

void PlayerReactionAreaChecker::Init() {

	// json適用
	ApplyJson();
}

void PlayerReactionAreaChecker::Update() {

	// 範囲内チェック
	for (auto& param : areaParams_) {

		// プレイヤーと攻撃対象の距離を取得
		float distance = SakuEngine::Math::GetDistance3D(*player_, *attackTarget_, false, true);
		// 範囲内チェック
		if (distance <= param.range) {

			param.isInRange = true;
		} else {
			
			param.isInRange = false;
		}
	}
}

void PlayerReactionAreaChecker::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(AreaReactionType::Count); ++i) {

		auto& param = areaParams_[i];

		const char* typeName = SakuEngine::EnumAdapter<AreaReactionType>::ToString(static_cast<AreaReactionType>(i));
		ImGui::SeparatorText(typeName);
		ImGui::PushID(typeName);

		ImGui::Text(std::format("IsInRange: {}", param.isInRange).c_str());
		ImGui::DragFloat("Range", &param.range, 0.1f);

		ImGui::PopID();
	}
}

bool PlayerReactionAreaChecker::IsInRange(AreaReactionType reactionType) const {

	uint32_t typeIndex = static_cast<uint32_t>(reactionType);
	return areaParams_[typeIndex].isInRange;
}

void PlayerReactionAreaChecker::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Player/ActionCombo/Methos/reactionAreaChecker.json", data)) {
		return;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(AreaReactionType::Count); ++i) {

		auto& param = areaParams_[i];
		const auto& paramJson = data["areaParams"][i];
		param.range = paramJson.value("range", 0.0f);
	}
}

void PlayerReactionAreaChecker::SaveJson() {

	Json data;

	for (uint32_t i = 0; i < static_cast<uint32_t>(AreaReactionType::Count); ++i) {

		const auto& param = areaParams_[i];
		Json paramJson;
		paramJson["range"] = param.range;
		data["areaParams"].push_back(paramJson);
	}

	SakuEngine::JsonAdapter::Save("Player/ActionCombo/Methos/reactionAreaChecker.json", data);
}
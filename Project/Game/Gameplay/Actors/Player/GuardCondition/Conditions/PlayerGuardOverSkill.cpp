#include "PlayerGuardOverSkill.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	PlayerGuardOverSkill classMethods
//============================================================================

bool PlayerGuardOverSkill::GetResult() const {

	const PlayerStats& stats = player_->GetStats();

	// 現在のスキルポイントが消費量を超えているか
	bool result = stats.skillCost < stats.currentSkillPoint;
	return result;
}

void PlayerGuardOverSkill::ImGui() {

	ImGui::Text(std::format("Result: {}", GetResult()).c_str());

	const PlayerStats& stats = player_->GetStats();

	ImGui::Text("currentSkillPoint: %d", stats.currentSkillPoint);
	ImGui::Text("skillCost:         %d", stats.skillCost);
	ImGui::Text("maxSkillPoint:     %d", stats.maxSkillPoint);
}
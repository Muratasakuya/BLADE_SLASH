#include "FollowCameraTargetResolver.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	FollowCameraTargetResolver classMethods
//============================================================================

void FollowCameraTargetResolver::Init() {

	// json適用
	ApplyJson();
}

void FollowCameraTargetResolver::Execute([[maybe_unused]] FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, [[maybe_unused]] float deltaTime) {

	// とりあえずプレイヤーの位置
	targetPos_ = dependencies_.player->GetTranslation();
}

void FollowCameraTargetResolver::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Config");

	ImGui::Text("TargetPos: (%.2f / %.2f / %.2f)", targetPos_.x, targetPos_.y, targetPos_.z);

	ImGui::SeparatorText("Parameters");
}

void FollowCameraTargetResolver::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/targetResolver.json", data)) {
		return;
	}
}

void FollowCameraTargetResolver::SaveJson() {

	Json data;

	SakuEngine::JsonAdapter::Save("Camera/Follow/targetResolver.json", data);
}
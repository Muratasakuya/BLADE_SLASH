#include "FollowCameraTargetResolver.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	FollowCameraTargetResolver classMethods
//============================================================================

void FollowCameraTargetResolver::Init() {

	// json適用
	ApplyJson();
}

void FollowCameraTargetResolver::Execute([[maybe_unused]] FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, [[maybe_unused]] float deltaTime) {

	// 基準位置と目標位置
	Vector3 basePos = dependencies_.player->GetTranslation();
	Vector3 targetPos = dependencies_.bossEnemy->GetTranslation();

	// とりあえずプレイヤーの位置
	targetPos_ = Vector3::Lerp(basePos, targetPos, distanceLerpRate_);	
}

void FollowCameraTargetResolver::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Config");

	ImGui::Text("TargetPos: (%.2f / %.2f / %.2f)", targetPos_.x, targetPos_.y, targetPos_.z);

	ImGui::SeparatorText("Parameters");

	ImGui::DragFloat("distanceLerpRate", &distanceLerpRate_, 0.01f);

	{
		LineRenderer* lineRenderer = LineRenderer::GetInstance();

		// 基準点と目標点を結ぶ線を描画
		lineRenderer->Get3D()->DrawLine(dependencies_.player->GetTranslation(),
			dependencies_.bossEnemy->GetTranslation(), Color::Green());
		// 目標追従位置を描画
		lineRenderer->Get3D()->DrawSphere(6, 4.0f, targetPos_, Color::Green());
	}
}

void FollowCameraTargetResolver::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Follow/targetResolver.json", data)) {
		return;
	}

	distanceLerpRate_ = data.value("distanceLerpRate", 0.0f);
}

void FollowCameraTargetResolver::SaveJson() {

	Json data;

	data["distanceLerpRate"] = distanceLerpRate_;

	JsonAdapter::Save("Camera/Follow/targetResolver.json", data);
}
#include "FollowCameraCalFov.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	FollowCameraCalFov classMethods
//============================================================================

void FollowCameraCalFov::Init() {

	// エリアチェッカー初期化
	areaChecker_ = std::make_unique<ObjectAreaChecker>();
	areaChecker_->Init("Camera/Follow/calFovAreaChecker.json");

	// json適用
	ApplyJson();
}

void FollowCameraCalFov::BindDependencies(const FollowCameraDependencies& dependencies) {

	// 依存設定
	dependencies_ = dependencies;
	// エリアチェッカーにオブジェクト設定
	areaChecker_->SetAnchor(dependencies_.player);
	areaChecker_->SetTarget(dependencies_.bossEnemy);
}

void FollowCameraCalFov::Execute([[maybe_unused]] FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, float deltaTime) {

	// エリアチェッカー更新
	areaChecker_->Update();

	// 画角を距離に応じて補間する
	calculatedFovY_ = std::lerp(minFovY_, maxFovY_, GetLerpT());
	calculatedFovY_ = std::clamp(calculatedFovY_, minFovY_, maxFovY_);
}

float FollowCameraCalFov::GetLerpT() const {

	// 最小距離、最大距離
	float minRange = areaChecker_->GetMinRange();
	float maxRange = areaChecker_->GetMaxRange();
	// 現在のプレイヤーと敵の距離
	float distance = SakuEngine::Math::GetDistance3D(*dependencies_.player, *dependencies_.bossEnemy, false, true);

	distance = std::clamp(distance, minRange, maxRange);

	// どれだけ最大距離と近いか
	float lerpT = std::clamp(distance / maxRange, 0.0f, 1.0f);
	return lerpT;
}

void FollowCameraCalFov::ImGui() {

	ImGui::PushID("FollowCameraCalFov");

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::SeparatorText("Parameters");

	ImGui::Text("calculated FovY: %.3f", calculatedFovY_);
	ImGui::Text("lerpT:           %.3f", GetLerpT());

	ImGui::DragFloat("minFovY", &minFovY_, 0.01f);
	ImGui::DragFloat("maxFovY", &maxFovY_, 0.01f);

	ImGui::PopID();

	ImGui::SeparatorText("Area Checker");
	ImGui::PushID("AreaChecker");

	areaChecker_->ImGui();
	ImGui::PopID();
}

void FollowCameraCalFov::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/calFov.json", data)) {
		return;
	}

	minFovY_ = data.value("minFovY_", 0.64f);
	maxFovY_ = data.value("maxFovY_", 0.64f);
}

void FollowCameraCalFov::SaveJson() {

	Json data;

	data["minFovY_"] = minFovY_;
	data["maxFovY_"] = maxFovY_;

	SakuEngine::JsonAdapter::Save("Camera/Follow/calFov.json", data);
}
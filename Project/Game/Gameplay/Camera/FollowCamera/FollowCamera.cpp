#include "FollowCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Editor/Camera/CameraEditor.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/MathLib/MathUtils.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

//============================================================================
//	FollowCamera classMethods
//============================================================================

void FollowCamera::LoadAnim() {

	if (isLoadedAnim_) {
		return;
	}

	SakuEngine::CameraEditor* editor = SakuEngine::CameraEditor::GetInstance();
	// プレイヤーの攻撃
	editor->LoadJson("Player/player3rdAttack.json");
	editor->LoadJson("Player/player4thAttack.json");
	editor->LoadJson("Player/playerParryRight.json");
	editor->LoadJson("Player/playerParryLeft.json");
	editor->LoadJson("Player/playerSkilMove.json");
	editor->LoadJson("Player/playerSkilJump.json");
	editor->LoadJson("Player/playerStunAttack.json");
	editor->LoadJson("SubPlayer/subPlayerAttack.json");

	// 読み込み済み
	isLoadedAnim_ = true;
}

void FollowCamera::BindDependencies(const FollowCameraDependencies& dependencies) {

	// 更新パスに依存オブジェクトを設定
	updatePass_->BindDependencies(dependencies);

	// 基準点、注視点を設定
	anchorObject_ = dependencies.player;
	lookAtTargetObject_ = dependencies.bossEnemy;
}

void FollowCamera::StartPlayerActionAnim(PlayerState state) {

	SakuEngine::CameraEditor* editor = SakuEngine::CameraEditor::GetInstance();

	// 状態毎に名前を取得
	std::string name{};
	switch (state) {
	case PlayerState::Attack_3rd: name = "player3rdAttack"; break;
	case PlayerState::Attack_4th: name = "player4thAttack"; break;
	case PlayerState::Parry:

		// 目標回転
		const SakuEngine::Quaternion baseTarget = lookAtTargetObject_->GetRotation();
		// y軸最短補間方向
		int direction = SakuEngine::Math::YawShortestDirection(transform_.rotation, baseTarget);

		// 0か-1なら左、+1は右からの視点のパリィアニメーションを行わせる
		name = (0 <= direction) ? "playerParryRight" : "playerParryLeft";
		break;
	}

	// 名前が設定されていればアニメーションを再生
	if (!name.empty()) {

		editor->StartAnim(name, true, true);
	}
}

void FollowCamera::StartPlayerActionAnim(const std::string& animName) {

	SakuEngine::CameraEditor* editor = SakuEngine::CameraEditor::GetInstance();

	// 名前が設定されていればアニメーションを再生
	if (!animName.empty()) {

		editor->StartAnim(animName, true, true);
	}
}

void FollowCamera::EndPlayerActionAnim() {

	SakuEngine::CameraEditor* editor = SakuEngine::CameraEditor::GetInstance();

	// アニメーションを終了させる
	editor->EndAnim();
}

void FollowCamera::Init() {

	displayFrustum_ = false;
	isLoadedAnim_ = false;

	// json適用
	ApplyJson();

	// 更新パス初期化
	updatePass_ = std::make_unique<FollowCameraUpdatePass>();
	updatePass_->Init();
}

void FollowCamera::Update() {

	// 更新パスの更新
	updatePass_->Update(*this);

	// エディターで更新しているときは処理しない
	if (isUpdateEditor_) {
		return;
	}

	// 行列更新
	BaseCamera::UpdateView(UpdateMode::Quaternion);
}

void FollowCamera::ImGui() {

	ImGui::SetWindowFontScale(0.8f);
	ImGui::PushItemWidth(itemWidth_);

	if (ImGui::BeginTabBar("FollowCameraTabs")) {
		if (ImGui::BeginTabItem("Init")) {
			if (ImGui::Button("Save Json", ImVec2(itemWidth_, 32.0f))) {

				SaveJson();
			}

			ImGui::DragFloat3("translation", &transform_.translation.x, 0.01f);
			ImGui::Text("rotation: %.2f, %.2f, %.2f, %.2f", transform_.rotation.x,
				transform_.rotation.y, transform_.rotation.z, transform_.rotation.w);

			ImGui::DragFloat("fovY", &fovY_, 0.01f);
			ImGui::DragFloat("nearClip", &nearClip_, 0.001f);
			ImGui::DragFloat("farClip", &farClip_, 1.0f);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("State")) {

			updatePass_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::SetWindowFontScale(1.0f);
	ImGui::PopItemWidth();
}

void FollowCamera::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Camera/Follow/initParameter.json", data)) {
		return;
	}

	fovY_ = SakuEngine::JsonAdapter::GetValue<float>(data, "fovY_");
	nearClip_ = SakuEngine::JsonAdapter::GetValue<float>(data, "nearClip_");
	farClip_ = SakuEngine::JsonAdapter::GetValue<float>(data, "farClip_");
}

void FollowCamera::SaveJson() {

	Json data;

	data["fovY_"] = fovY_;
	data["nearClip_"] = nearClip_;
	data["farClip_"] = farClip_;

	SakuEngine::JsonAdapter::Save("Camera/Follow/initParameter.json", data);
}
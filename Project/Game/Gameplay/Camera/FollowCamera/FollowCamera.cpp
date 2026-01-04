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
#include <Engine/Config.h>
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
	// プレイヤーの攻撃、全て左視点で作成済み
	editor->LoadJson("Player/player3rdAttack.json");
	editor->LoadJson("Player/player4thAttack.json");
	editor->LoadJson("Player/playerParry.json");
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
	case PlayerState::Parry: name = "playerParry"; break;
	}

	// 目標回転が基準点から見て左か右か
	AnchorToDirection2D lookYawDirection = SakuEngine::Math::YawSideFromPos(
		transform_.translation, transform_.rotation, lookAtTargetObject_->GetTranslation());

	// エディター反転設定
	// 位置に応じて反転するかしないかを決定する
	editorInverseSetting_.isInversePos = lookYawDirection == AnchorToDirection2D::Right;
	editorInverseSetting_.isInverseRotation = lookYawDirection == AnchorToDirection2D::Right;

	// 名前が設定されていればアニメーションを再生
	if (!name.empty()) {

		editor->StartAnim(name, true, true, editorInverseSetting_);
	}
}

void FollowCamera::StratPlayerActionAnimString(const std::string& animName) {

	SakuEngine::CameraEditor* editor = SakuEngine::CameraEditor::GetInstance();

	// 目標回転が基準点から見て左か右か
	AnchorToDirection2D lookYawDirection{};
	// yaw方向決定
	const float yawDelta = SakuEngine::Math::YawSignedDelta(transform_.rotation, lookAtTargetObject_->GetRotation());
	if (std::abs(yawDelta) <= Config::kEpsilon) {

		// どちらでも良いので右にする
		lookYawDirection = AnchorToDirection2D::Right;
	} else {

		// 最短方向
		lookYawDirection = (0.0f < yawDelta) ? AnchorToDirection2D::Right : AnchorToDirection2D::Left;
	}

	// エディター反転設定
	// 位置に応じて反転するかしないかを決定する
	editorInverseSetting_.isInversePos = lookYawDirection == AnchorToDirection2D::Right;
	editorInverseSetting_.isInverseRotation = lookYawDirection == AnchorToDirection2D::Right;

	// 名前が設定されていればアニメーションを再生
	if (!animName.empty()) {

		editor->StartAnim(animName, true, true, editorInverseSetting_);
	}
}

void FollowCamera::Init() {

	displayFrustum_ = false;
	isLoadedAnim_ = false;

	// json適用
	ApplyJson();

	// 更新パス初期化
	updatePass_ = std::make_unique<FollowCameraUpdatePass>();
	updatePass_->Init();

	// 反転軸設定
	// 位置反転軸設定
	editorInverseSetting_.inversePosAxisMap[SakuEngine::Math::Axis::X] = true;
	editorInverseSetting_.inversePosAxisMap[SakuEngine::Math::Axis::Y] = false;
	editorInverseSetting_.inversePosAxisMap[SakuEngine::Math::Axis::Z] = false;
	// 回転反転軸設定
	editorInverseSetting_.inverseRotateAxisMap[SakuEngine::Math::Axis::X] = true;
	editorInverseSetting_.inverseRotateAxisMap[SakuEngine::Math::Axis::Y] = false;
	editorInverseSetting_.inverseRotateAxisMap[SakuEngine::Math::Axis::Z] = false;
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

			ImGui::Text(std::format("isUpdateEditor: {}", isUpdateEditor_).c_str());
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("UpdatePass")) {

			updatePass_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("EditorCamera")) {

			ImGui::Text(std::format("isInversePos:      {}", editorInverseSetting_.isInversePos).c_str());
			ImGui::Text(std::format("isInverseRotation: {}", editorInverseSetting_.isInverseRotation).c_str());
			ImGui::Checkbox("isRotationFollowPosAxis", &editorInverseSetting_.isRotationFollowPosAxis);
			for (auto& flag : editorInverseSetting_.inversePosAxisMap) {

				std::string label = "InversePosAxis:" + std::string(SakuEngine::EnumAdapter<SakuEngine::Math::Axis>::ToString(flag.first));
				ImGui::Checkbox(label.c_str(), &flag.second);
			}
			for (auto& flag : editorInverseSetting_.inverseRotateAxisMap) {

				std::string label = "InverseRotateAxis:" + std::string(SakuEngine::EnumAdapter<SakuEngine::Math::Axis>::ToString(flag.first));
				ImGui::Checkbox(label.c_str(), &flag.second);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Runtime")) {

			// 目標回転が基準点から見て左か右か
			{
				AnchorToDirection2D lookYawDirection = SakuEngine::Math::YawSideFromPos(
					transform_.translation, transform_.rotation, lookAtTargetObject_->GetTranslation());
				ImGui::Text(SakuEngine::EnumAdapter<AnchorToDirection2D>::ToString(lookYawDirection));
			}
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
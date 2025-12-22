#include "FollowCameraStateMachine.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// state
#include <Game/Gameplay/Camera/FollowCamera/StateMachine/States/FollowCameraFollowState.h>

// imgui
#include <imgui.h>

//============================================================================
//	FollowCameraStateMachine classMethods
//============================================================================

namespace {

	// 全てのFollowCameraStateに対して関数を実行
	// Countは除外
	template <typename Fn>
	static void ForEachState(Fn&& function) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(FollowCameraState::Count); ++i) {
			FollowCameraState state = static_cast<FollowCameraState>(i);
			if (state == FollowCameraState::Count) {
				continue;
			}
			function(state);
		}
	}
}

void FollowCameraStateMachine::Init(FollowCamera* camera) {

	camera_ = nullptr;
	camera_ = camera;

	// 各状態の追加
	auto& machine = BaseStateController::GetMachine();
	machine.Add<FollowCameraFollowState>(FollowCameraState::Follow);

	ForEachState([&](FollowCameraState state) {
		if (!machine.Has(state)) {
			return;
		}
		// 状態遷移対象を設定
		machine.Get(state).SetFollowCamera(camera);
		});

	// json適用
	ApplyJson();

	// 最初の状態を設定
	machine.SetEnter(FollowCameraState::Follow);
	editingState_ = FollowCameraState::Follow;
}

void FollowCameraStateMachine::SetAnchorObject(const SakuEngine::GameObject3D* anchor) {

	auto& machine = BaseStateController::GetMachine();

	// 各状態に基準点をセット
	ForEachState([&](FollowCameraState state) {
		if (!machine.Has(state)) {
			return;
		}
		// 状態遷移対象を設定
		machine.Get(state).SetAnchorObject(anchor);
		});
}

void FollowCameraStateMachine::SetLookAtTargetObject(const SakuEngine::GameObject3D* lookAtTarget) {

	auto& machine = BaseStateController::GetMachine();

	// 各状態に基準点をセット
	ForEachState([&](FollowCameraState state) {
		if (!machine.Has(state)) {
			return;
		}
		// 状態遷移対象を設定
		machine.Get(state).SetLookAtTargetObject(lookAtTarget);
		});
}

void FollowCameraStateMachine::Update() {

	// 状態遷移、更新処理
	BaseStateController::Tick();
}

void FollowCameraStateMachine::DecideExternalTransition() {
}

void FollowCameraStateMachine::ImGui() {

	if (ImGui::Button("Save##StateJson")) {

		SaveJson();
	}

	auto& machine = BaseStateController::GetMachine();

	ImGui::SeparatorText("State");

	SakuEngine::EnumAdapter<FollowCameraState>::Combo("##StateCombo", &editingState_);
	if (machine.Has(editingState_)) {

		machine.Get(editingState_).ImGui();
	}
}

void FollowCameraStateMachine::UpdateInitialSettings() {

	auto& machine = BaseStateController::GetMachine();
	if (auto* followState = static_cast<FollowCameraFollowState*>(&machine.Get(FollowCameraState::Follow))) {

		followState->UpdateInitialSettings();
	}
}

void FollowCameraStateMachine::WarmStartFollow() {

	auto& machine = BaseStateController::GetMachine();
	if (auto* followState = static_cast<FollowCameraFollowState*>(&machine.Get(FollowCameraState::Follow))) {

		followState->SnapToCamera();
	}
}

bool FollowCameraStateMachine::IsFinishedHandoffBlend() {

	auto& machine = BaseStateController::GetMachine();
	if (auto* followState = static_cast<FollowCameraFollowState*>(&machine.Get(FollowCameraState::Follow))) {

		return followState->IsFinishedHandoffBlend();
	}
	return false;
}

void FollowCameraStateMachine::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck(kStateJsonPath_, data)) {
		return;
	}

	auto& machine = BaseStateController::GetMachine();

	// 各状態に対して処理
	ForEachState([&](FollowCameraState state) {

		// キー取得
		const auto& key = SakuEngine::EnumAdapter<FollowCameraState>::ToString(state);
		if (!machine.Has(state) || !data.contains(key)) {
			return;
		}
		machine.Get(state).ApplyJson(data[key]);
		});
}

void FollowCameraStateMachine::SaveJson() {

	Json data;

	auto& machine = BaseStateController::GetMachine();

	// 各状態に対して処理
	ForEachState([&](FollowCameraState state) {
		if (!machine.Has(state)) {
			return;
		}
		machine.Get(state).SaveJson(data[SakuEngine::EnumAdapter<FollowCameraState>::ToString(state)]);
		});

	SakuEngine::JsonAdapter::Save(kStateJsonPath_, data);
}
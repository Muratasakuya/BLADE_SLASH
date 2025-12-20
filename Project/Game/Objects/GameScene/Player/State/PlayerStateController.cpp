#include "PlayerStateController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Input/Input.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

// inputDevice
#include <Game/Objects/GameScene/Player/Input/Device/PlayerKeyInput.h>
#include <Game/Objects/GameScene/Player/Input/Device/PlayerGamePadInput.h>

// state
#include <Game/Objects/GameScene/Player/State/States/PlayerIdleState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerWalkState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerDashState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAvoidSatate.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAttack_1stState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAttack_2ndState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAttack_3rdState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAttack_4thState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerSkilAttackState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerParryState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerFalterState.h>

//============================================================================
//	PlayerStateController classMethods
//============================================================================

namespace {

	// 全てのPlayerStateに対して関数を実行
	// NoneとCountは除外
	template<class Fn>
	static void ForEachPlayerState(Fn&& function) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(PlayerState::Count); ++i) {
			PlayerState state = static_cast<PlayerState>(i);
			if (state == PlayerState::None || state == PlayerState::Count) {
				continue;
			}
			function(state);
		}
	}
}

void PlayerStateController::Init(Player* owner) {

	player_ = nullptr;
	player_ = owner;

	// 入力クラスを初期化
	inputTransitionPlanner_.Init();
	SakuEngine::InputMapper<PlayerInputAction>* inputMapper = inputTransitionPlanner_.GetInputMapper();

	// 各状態の追加
	auto& machine = BaseStateController::GetMachine();
	machine.Add<PlayerIdleState>(PlayerState::Idle);
	machine.Add<PlayerWalkState>(PlayerState::Walk, inputMapper);
	machine.Add<PlayerDashState>(PlayerState::Dash, inputMapper);
	machine.Add<PlayerAvoidSatate>(PlayerState::Avoid);
	machine.Add<PlayerAttack_1stState>(PlayerState::Attack_1st);
	machine.Add<PlayerAttack_2ndState>(PlayerState::Attack_2nd);
	machine.Add<PlayerAttack_3rdState>(PlayerState::Attack_3rd);
	machine.Add<PlayerAttack_4thState>(PlayerState::Attack_4th);
	machine.Add<PlayerSkilAttackState>(PlayerState::SkilAttack);
	machine.Add<PlayerParryState>(PlayerState::Parry, inputMapper);
	machine.Add<PlayerFalterState>(PlayerState::Falter);

	ForEachPlayerState([&](PlayerState state) {
		if (!machine.Has(state)) {
			return;
		}
		// 状態遷移対象を設定
		machine.Get(state).SetPlayer(owner);
		// エフェクト生成
		machine.Get(state).CreateEffect();
		});

	// json適応
	ApplyJson();

	// 初期状態を設定
	machine.SetEnter(PlayerState::Idle);

	// 初期化
	owner->GetAttackCollision()->SetEnterState(PlayerState::Idle);
	currentEnterTime_ = SakuEngine::GameTimer::GetTotalTime();
	lastEnterTime_[PlayerState::Idle] = currentEnterTime_;
	inputTransitionPlanner_.Reset();
	parrySystem_.Reset();
}

void PlayerStateController::SetBossEnemy(BossEnemy* bossEnemy) {

	auto& machine = BaseStateController::GetMachine();

	// 各状態にbossEnemyをセット
	ForEachPlayerState([&](PlayerState state) {
		if (!machine.Has(state)) {
			return;
		}
		machine.Get(state).SetBossEnemy(bossEnemy);
		});
	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;
}

void PlayerStateController::SetFollowCamera(FollowCamera* followCamera) {

	auto& machine = BaseStateController::GetMachine();

	// 各状態にfollowCameraをセット
	// 必要なクラスを設定
	ForEachPlayerState([&](PlayerState state) {
		if (!machine.Has(state)) {
			return;
		}
		machine.Get(state).SetFollowCamera(followCamera);
		});
}

void PlayerStateController::SetForcedState(PlayerState forcedState) {

	auto& machine = BaseStateController::GetMachine();
	PlayerState currentState = machine.GetCurrentId();

	// リクエストを破棄
	queued_.reset();
	machine.Request(currentState);

	// 連続で同じ状態への強制遷移かどうか
	bool restartSame = (currentState == forcedState) && conditions_.at(forcedState).enableInARowForceState;

	// 強制遷移実行
	machine.Force(forcedState, restartSame);

	// タイム更新と通知
	currentEnterTime_ = SakuEngine::GameTimer::GetTotalTime();
	lastEnterTime_[forcedState] = currentEnterTime_;
	player_->GetAttackCollision()->SetEnterState(forcedState);
}

void PlayerStateController::RequestFalterState() {

	// 現在の状態が攻撃を受けたら怯む状態なら遷移させる
	if (conditions_.at(BaseStateController::GetMachine().GetCurrentId()).isArmor) {
		// 怯み無効状態なら遷移させない
		return;
	}

	// 怯み状態に遷移させる 
	SetForcedState(PlayerState::Falter);
}

void PlayerStateController::Update() {

	auto& machine = BaseStateController::GetMachine();

	// 全ての状態の常に行う前処理
	ForEachPlayerState([&](PlayerState state) {
		if (machine.Has(state)) {

			machine.Get(state).BeginUpdateAlways();
		}
		});

	// 状態遷移、更新処理
	BaseStateController::Tick();

	// 状態が変化したかどうかの判定と処理
	OnStateChanged();

	// パリィの状態管理
	parrySystem_.PostStateUpdate(machine.GetCurrentId(), machine.GetCurrent().GetCanExit());

	// 全ての状態の常に行う更新処理
	ForEachPlayerState([&](PlayerState state) {
		if (machine.Has(state)) {

			machine.Get(state).UpdateAlways();
		}
		});
}

void PlayerStateController::DecideExternalTransition() {

	auto& machine = BaseStateController::GetMachine();

	// 入力遷移
	inputTransitionPlanner_.Update(*this, player_->GetStats());

	// パリィ状態への遷移判定
	parrySystem_.UpdateDuringExternalTransition(*this, *bossEnemy_);

	// 何か予約設定されて入れば状態遷移させる
	if (queued_) {
		bool canInterrupt = false;
		if (auto it = conditions_.find(*queued_); it != conditions_.end()) {

			canInterrupt = it->second.CheckInterruptableByState(machine.GetCurrentId());
		}
		if ((machine.GetCurrent().GetCanExit() || canInterrupt) && CanTransition(*queued_, !canInterrupt)) {

			machine.Request(*queued_);
			queued_.reset();
		}
	}
}

void PlayerStateController::OnStateChanged() {

	const auto& machine = BaseStateController::GetMachine();

	PlayerState currentState = machine.GetCurrentId();
	// 状態が変化した場合の処理
	if (machine.GetPreviousId() != currentState) {

		// タイム更新と通知
		currentEnterTime_ = SakuEngine::GameTimer::GetTotalTime();
		lastEnterTime_[currentState] = currentEnterTime_;
		player_->GetAttackCollision()->SetEnterState(currentState);
	}
}

void PlayerStateController::SetParryAllowAttack(bool allowAttack) {

	auto& machine = BaseStateController::GetMachine();
	// 現在がParry状態でなければ何もしない
	if (machine.GetCurrentId() != PlayerState::Parry) {
		return;
	}
	// ParryStateに攻撃許可フラグをセット
	if (PlayerParryState* parryState = static_cast<PlayerParryState*>(&machine.GetCurrent())) {

		parryState->SetAllowAttack(allowAttack);
	}
}

bool PlayerStateController::Request(PlayerState requestState) {

	auto& machine = BaseStateController::GetMachine();

	PlayerState currentState = machine.GetCurrentId();
	// 同じ状態なら何もしない
	if (requestState == currentState) {
		return false;
	}
	if (queued_ && *queued_ == requestState) {
		return true;
	}

	// 遷移可能かチェック
	if (!CanTransition(requestState, false)) {
		return false;
	}

	bool canInterrupt = false;
	// 現在stateが抜けられない場合、強制キャンセル可能かチェック
	if (auto it = conditions_.find(requestState); it != conditions_.end()) {
		canInterrupt = it->second.CheckInterruptableByState(currentState);
	}

	// 現在の状態が終了できなければ予約する
	if (!machine.GetCurrent().GetCanExit() && !canInterrupt) {

		queued_ = requestState;

	} else {

		// 直接遷移させる
		machine.Request(requestState);
	}
	return true;
}

bool PlayerStateController::CanTransition(PlayerState next, bool viaQueue) const {

	PlayerState currentState = BaseStateController::GetMachine().GetCurrentId();

	const auto it = conditions_.find(next);
	if (it == conditions_.end()) {
		return true;
	}

	const PlayerStateCondition& condition = it->second;
	const float totalTime = SakuEngine::GameTimer::GetTotalTime();

	// クールタイムの処理
	auto itTime = lastEnterTime_.find(next);
	// クールタイムが終わっていなければ遷移不可
	if (itTime != lastEnterTime_.end() && totalTime - itTime->second < condition.coolTime) {

		return false;
	}

	// 強制キャンセルを行えるか判定
	if (!viaQueue) {
		if (!condition.interruptableBy.empty()) {
			const bool cancel = std::ranges::find(
				condition.interruptableBy, currentState) != condition.interruptableBy.end();
			if (!cancel) {

				return false;
			}
		}
	}

	// 遷移可能な前状態かチェック
	if (!condition.allowedPreState.empty()) {
		const bool ok = std::ranges::find(condition.allowedPreState, currentState) !=
			condition.allowedPreState.end();
		if (!ok) {

			return false;
		}
	}

	// コンボ入力判定
	if (!viaQueue && condition.chainInputTime > 0.0f) {
		if (totalTime - currentEnterTime_ > condition.chainInputTime) {
			return false;
		}
	}
	return true;
}

void PlayerStateController::ImGui() {

	// tool
	ImGui::Text("Current : %s", SakuEngine::EnumAdapter<PlayerState>::ToString(
		BaseStateController::GetMachine().GetCurrentId()));
	ImGui::SameLine();
	if (ImGui::Button("Save##StateJson")) {
		SaveJson();
	}

	// main
	if (ImGui::BeginTabBar("PStateTabs")) {

		// ---- Runtime -------------------------------------------------
		if (ImGui::BeginTabItem("Runtime")) {

			ImGui::Text("Enter Time   : %.2f", currentEnterTime_);
			ImGui::Text("Queued State : %s", queued_ ? SakuEngine::EnumAdapter<PlayerState>::ToString(*queued_) : "None");

			ImGui::EndTabItem();
		}

		// ---- States --------------------------------------------------
		if (ImGui::BeginTabItem("States")) {
			ImGui::BeginChild("StateList", ImVec2(140, 0), true);
			for (uint32_t i = 0; i < SakuEngine::EnumAdapter<PlayerState>::GetEnumCount(); ++i) {

				bool selected = (editingStateIndex_ == static_cast<int>(i));
				if (ImGui::Selectable(SakuEngine::EnumAdapter<PlayerState>::GetEnumName(i), selected)) {

					editingStateIndex_ = static_cast<int>(i);
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("StateDetail", ImVec2(0, 0), true);

			if (PlayerState::None != static_cast<PlayerState>(editingStateIndex_) &&
				PlayerState::Count != static_cast<PlayerState>(editingStateIndex_)) {
				if (PlayerIState* state = static_cast<PlayerIState*>(&BaseStateController::GetMachine().Get(
					static_cast<PlayerState>(editingStateIndex_)))) {

					state->ImGui();
				}
			}
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		// ---- Conditions ---------------------------------------------
		if (ImGui::BeginTabItem("Conditions")) {
			ImGui::Combo("Edit##cond-state", &comboIndex_,
				SakuEngine::EnumAdapter<PlayerState>::GetEnumArray().data(),
				static_cast<int>(SakuEngine::EnumAdapter<PlayerState>::GetEnumCount()));

			PlayerState state = static_cast<PlayerState>(comboIndex_);
			PlayerStateCondition& cond = conditions_[state];

			ImGui::Checkbox("isArmor", &cond.isArmor);
			ImGui::Checkbox("enableInARowForceState", &cond.enableInARowForceState);
			ImGui::DragFloat("CoolTime", &cond.coolTime, 0.01f, 0.0f);
			ImGui::DragFloat("InputWindow", &cond.chainInputTime, 0.01f, 0.0f);

			// Allowed / Interruptable をテーブルで
			if (ImGui::BeginTable("CondTable", 3, ImGuiTableFlags_Borders)) {
				ImGui::TableSetupColumn("State");
				ImGui::TableSetupColumn("Allowed");
				ImGui::TableSetupColumn("Interrupt");
				ImGui::TableHeadersRow();

				for (int i = 0; i < SakuEngine::EnumAdapter<PlayerState>::GetEnumCount(); ++i) {
					PlayerState s = static_cast<PlayerState>(i);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(SakuEngine::EnumAdapter<PlayerState>::GetEnumName(i));

					// ---- Allowed 列 -------------------------------------------------
					ImGui::TableNextColumn();
					{
						bool allowed = std::ranges::find(cond.allowedPreState, s)
							!= cond.allowedPreState.end();
						std::string id = "##allow_" + std::to_string(i);
						if (ImGui::Checkbox(id.c_str(), &allowed)) {
							if (allowed) {
								cond.allowedPreState.push_back(s);
							} else {
								std::erase(cond.allowedPreState, s);
							}
						}
					}

					// ---- Interrupt 列 -----------------------------------------------
					ImGui::TableNextColumn();
					{
						bool intr = std::ranges::find(cond.interruptableBy, s)
							!= cond.interruptableBy.end();
						std::string id = "##intr_" + std::to_string(i);
						if (ImGui::Checkbox(id.c_str(), &intr)) {
							if (intr) {
								cond.interruptableBy.push_back(s);
							} else {
								std::erase(cond.interruptableBy, s);
							}
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PlayerStateController::ApplyJson() {

	// state
	{
		Json data;
		if (!SakuEngine::JsonAdapter::LoadCheck(kStateJsonPath_, data)) {
			return;
		}

		auto& machine = BaseStateController::GetMachine();

		// 各状態に対して処理
		ForEachPlayerState([&](PlayerState state) {

			// キー取得
			const auto& key = SakuEngine::EnumAdapter<PlayerState>::ToString(state);
			if (!machine.Has(state) || !data.contains(key)) {
				return;
			}
			machine.Get(state).ApplyJson(data[key]);

			const Json& condRoot = data["Conditions"];
			if (!condRoot.contains(key)) {
				return;
			}

			PlayerStateCondition condition{};
			condition.FromJson(condRoot[key]);
			conditions_[state] = std::move(condition);
			});
	}
}

void PlayerStateController::SaveJson() {

	Json data;

	auto& machine = BaseStateController::GetMachine();

	// 各状態に対して処理
	ForEachPlayerState([&](PlayerState state) {
		if (!machine.Has(state)) {
			return;
		}
		machine.Get(state).SaveJson(data[SakuEngine::EnumAdapter<PlayerState>::ToString(state)]);
		});

	Json& condRoot = data["Conditions"];
	for (auto& [state, cond] : conditions_) {
		if (state == PlayerState::None) {
			continue;
		}

		cond.ToJson(condRoot[SakuEngine::EnumAdapter<PlayerState>::ToString(state)]);
	}

	SakuEngine::JsonAdapter::Save(kStateJsonPath_, data);
}
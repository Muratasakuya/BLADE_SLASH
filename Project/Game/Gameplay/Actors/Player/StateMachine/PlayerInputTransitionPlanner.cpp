#include "PlayerInputTransitionPlanner.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Input/Input.h>
#include <Game/Gameplay/Actors/Player/StateMachine/PlayerStateController.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>

// inputDevice
#include <Game/Gameplay/Actors/Player/Input/Devices/PlayerKeyInput.h>
#include <Game/Gameplay/Actors/Player/Input/Devices/PlayerGamePadInput.h>

//============================================================================
//	PlayerInputTransitionPlanner classMethods
//============================================================================

void PlayerInputTransitionPlanner::Init() {

	// 入力クラスを初期化
	SakuEngine::Input* input = SakuEngine::Input::GetInstance();
	inputMapper_ = std::make_unique<SakuEngine::InputMapper<PlayerInputAction>>();
	inputMapper_->AddDevice(std::make_unique<PlayerGamePadInput>(input));
	//inputMapper_->AddDevice(std::make_unique<PlayerKeyInput>(input));

	// リセット
	Reset();
}

void PlayerInputTransitionPlanner::Reset() {

	// ダッシュ入力状態リセット
	isDashInput_ = false;
}

void PlayerInputTransitionPlanner::Update(PlayerStateController& controller, const PlayerStats& stats) {

	auto& machine = controller.GetMachine();
	// 現在の状態を取得
	PlayerState currentState = machine.GetCurrentId();

	// コンボ中は判定をスキップする
	bool inCombat = IsCombatState(currentState);
	bool actionLocked = (inCombat && !machine.GetCurrent().GetCanExit()) || (inCombat && IsInChain(controller));

	// 移動方向
	SakuEngine::Vector2 move(inputMapper_->GetVector(PlayerInputAction::MoveX),
		inputMapper_->GetVector(PlayerInputAction::MoveZ));

	// 動いたかどうか判定
	bool isMove = move.Length() > Config::kEpsilon;
	// 歩き、待機状態の状態遷移
	{
		if (!actionLocked && currentState != PlayerState::Dash) {

			// 移動していた場合は歩き、していなければ待機状態のまま
			if (isMove) {

				controller.Request(PlayerState::Walk);
			} else {

				controller.Request(PlayerState::Idle);
			}
		}
	}

	// ダッシュ、攻撃の状態遷移
	{
		// ダッシュ入力があったかどうか
		if (inputMapper_->IsTriggered(PlayerInputAction::Dash)) {

			isDashInput_ = true;
		}

		// 移動していなければダッシュ入力をリセット
		if (!isMove) {

			isDashInput_ = false;
		}

		// 移動している時にダッシュ入力があればダッシュ状態に遷移
		if (isMove && isDashInput_) {

			controller.Request(PlayerState::Dash);
		} else if (!isMove && currentState == PlayerState::Dash) {

			// 移動が止まったらダッシュ終了
			controller.Request(PlayerState::Idle);
		}

		if (inputMapper_->IsTriggered(PlayerInputAction::Attack)) {

			if (currentState == PlayerState::Attack_1st) {

				controller.Request(PlayerState::Attack_2nd);
			}
			// 2段 -> 3段
			else if (currentState == PlayerState::Attack_2nd) {

				controller.Request(PlayerState::Attack_3rd);
			}
			// 3段 -> 4段
			else if (currentState == PlayerState::Attack_3rd) {

				controller.Request(PlayerState::Attack_4th);
			}
			// 1段目
			else {

				// 1段目の攻撃
				controller.Request(PlayerState::Attack_1st);
			}

			// ダッシュ入力をリセット
			isDashInput_ = false;
			return;
		}

		// スキル攻撃
		// スキルポイントが足りていてスキル入力があればスキル攻撃状態に遷移
		if (stats.skillCost <= stats.currentSkillPoint &&
			inputMapper_->IsTriggered(PlayerInputAction::Skill)) {

			controller.Request(PlayerState::SkilAttack);
			return;
		}
	}

	// 回避入力、回避直後は状態を受け付けない
	if (!isDashInput_ && inputMapper_->IsTriggered(PlayerInputAction::Avoid)) {

		controller.Request(PlayerState::Avoid);
		return;
	}

	// パリィの入力判定、攻撃を受けた、受けているときは無効
	controller.parrySystem_.TryReserveByInput(currentState, *controller.bossEnemy_, *inputMapper_);

	// ダッシュ中にダッシュ入力があればダッシュ状態を再度強制遷移させる
	if (currentState == PlayerState::Dash && inputMapper_->IsTriggered(PlayerInputAction::Dash)) {

		controller.SetForcedState(PlayerState::Dash);
	}
}

bool PlayerInputTransitionPlanner::IsInChain(PlayerStateController& controller) const {

	auto& machine = controller.GetMachine();

	auto it = controller.conditions_.find(machine.GetCurrentId());
	if (it == controller.conditions_.end()) {
		return false;
	}

	float elapsed = SakuEngine::GameTimer::GetTotalTime() - controller.currentEnterTime_;
	return (it->second.chainInputTime > 0.0f) && (elapsed <= it->second.chainInputTime);
}

bool PlayerInputTransitionPlanner::IsCombatState(PlayerState state) const {

	switch (state) {
	case PlayerState::Attack_1st:
	case PlayerState::Attack_2nd:
	case PlayerState::Attack_3rd:
	case PlayerState::Attack_4th:
	case PlayerState::SkilAttack:
	case PlayerState::Parry:
		return true;
	default:
		return false;
	}
}
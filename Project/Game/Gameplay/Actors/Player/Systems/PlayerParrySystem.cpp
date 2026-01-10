#include "PlayerParrySystem.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/StateMachine/PlayerStateController.h>

//============================================================================
//	PlayerParrySystem classMethods
//============================================================================

void PlayerParrySystem::Session::Reset() {

	// 状態リセット
	active = false;
	reserved = false;
	total = 0;
	done = 0;
	reservedStart = 0.0f;
}

void PlayerParrySystem::Reset() {

	// セッションリセット
	session_.Reset();
}

void PlayerParrySystem::TryReserveByInput(PlayerStateController& controller,
	const BossEnemy& bossEnemy, const SakuEngine::InputMapper<PlayerInputAction>& inputMapper) {

	// 攻撃を受けた、受けているときは無効、または既にパリィ処理中なら何もしない
	PlayerState current = controller.GetCurrentState();
	if (current == PlayerState::Falter ||
		current == PlayerState::Parry ||
		current == PlayerState::ParryWait) {
		return;
	}
	if (session_.active) {
		return;
	}

	// 現在の入力タイプ
	InputType inputType = SakuEngine::Input::GetInstance()->GetType();

	// パリィ入力が無ければ何もしない
	if (!inputMapper.IsTriggered(PlayerInputAction::Parry, inputType)) {
		return;
	}

	// パリィ可能かチェック
	const ParryParameter& parryParam = bossEnemy.GetParryParam();
	if (!parryParam.canParry) {
		return;
	}

	// 予約が入ったタイミングでパリィ待機状態に遷移させる
	controller.SetForcedState(PlayerState::ParryWait);

	// 入力があればパリィ処理を予約する
	session_ = {};
	session_.done = 0;
	session_.active = true;
	session_.reserved = true;
	// パリィ回数設定
	session_.total = std::max<uint32_t>(1, parryParam.continuousCount);
	session_.reservedStart = SakuEngine::GameTimer::GetTotalTime();
}

void PlayerParrySystem::UpdateDuringExternalTransition(PlayerStateController& controller, BossEnemy& bossEnemy) {

	// 敵がパリィ可能かどうかチェック、タイミング消費
	if (session_.active && session_.reserved && bossEnemy.ConsumeParryTiming()) {

		// パリィ回数を進める
		++session_.done;

		// 最後のパリィかどうか
		bool isLast = (session_.done >= session_.total);

		// パリィ状態に強制遷移させる
		controller.SetForcedState(PlayerState::Parry);

		// 最後なら攻撃可能
		controller.SetParryAllowAttack(isLast);

		// 継続するなら次のタイミング待ち
		session_.reserved = !isLast;
		session_.reservedStart = SakuEngine::GameTimer::GetTotalTime();
		return;
	}

	// パリィ受付をしていないなら初期化
	if (session_.active && !bossEnemy.GetParryParam().canParry) {

		session_.Reset();
	}
}

void PlayerParrySystem::PostStateUpdate(PlayerState currentState, bool canExit) {

	// パリィ状態が終了できるタイミングになったかどうか
	if (currentState == PlayerState::Parry && canExit) {

		if (session_.done < session_.total) {

			// 次のタイミング待ち開始
			session_.reservedStart = SakuEngine::GameTimer::GetTotalTime();
		} else {

			// 全パリィ完了
			session_.Reset();
		}
	}
}
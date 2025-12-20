#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/InputMapper.h>
#include <Game/Gameplay/Actors/Player/Structure/PlayerStructures.h>
#include <Game/Gameplay/Actors/Player/Input/Actions/PlayerInputAction.h>

// front
class BossEnemy;
class PlayerStateController;

//============================================================================
//	PlayerParrySystem class
//	ボスのパリィパラメータを見て、プレイヤーのパリィ状態を管理する
//============================================================================
class PlayerParrySystem {
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// パリィ処理状態
	struct Session {

		bool active = false;   // 処理中か
		bool reserved = false; // タイミング待ち
		uint32_t total = 0;    // 連続回数
		uint32_t done = 0;     // 処理済み回数
		float reservedStart = 0.0f;

		void Reset();
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerParrySystem() = default;
	~PlayerParrySystem() = default;

	// パリィ状態をリセットする
	void Reset();

	// 入力によるパリィ予約を開始する
	void TryReserveByInput(PlayerState currentState, const BossEnemy& bossEnemy,
		const SakuEngine::InputMapper<PlayerInputAction>& inputMapper);

	// 外部遷移判定中に、ボスのパリィタイミング消費を見てパリィ状態へ遷移させる
	void UpdateDuringExternalTransition(PlayerStateController& controller, BossEnemy& bossEnemy);

	// パリィ状態の終了タイミングでセッションを更新する
	void PostStateUpdate(PlayerState currentState, bool canExit);

	//--------- accessor -----------------------------------------------------

	bool IsActive() const { return session_.active; }
	const Session& GetSession() const { return session_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Session session_{};
};
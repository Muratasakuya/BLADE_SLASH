#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/InputMapper.h>
#include <Game/Gameplay/Actors/Player/Structure/PlayerStructures.h>
#include <Game/Gameplay/Actors/Player/Input/Actions/PlayerInputAction.h>

// front
class PlayerStateController;

//============================================================================
//	PlayerInputTransitionPlanner class
//	プレイヤーの入力による状態遷移を行うクラス
//============================================================================
class PlayerInputTransitionPlanner {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerInputTransitionPlanner() = default;
	~PlayerInputTransitionPlanner() = default;

	// 初期化
	void Init();

	// リセット
	void Reset();

	// 入力に応じた状態遷移
	void Update(PlayerStateController& controller, const PlayerStats& stats);

	//--------- accessor -----------------------------------------------------

	// 入力マッパーを取得
	SakuEngine::InputMapper<PlayerInputAction>* GetInputMapper() const { return inputMapper_.get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 入力
	std::unique_ptr<SakuEngine::InputMapper<PlayerInputAction>> inputMapper_;

	// ダッシュ入力中か
	bool isDashInput_ = false;

	//--------- functions ----------------------------------------------------

	// コンボ中か
	bool IsInChain(PlayerStateController& controller) const;
	bool IsCombatState(PlayerState state) const;
};
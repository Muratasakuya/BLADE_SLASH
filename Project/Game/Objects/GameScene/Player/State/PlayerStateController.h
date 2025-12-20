#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/BaseStateController.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Game/Objects/GameScene/Player/State/PlayerStateConfig.h>
#include <Game/Objects/GameScene/Player/State/PlayerInputTransitionPlanner.h>
#include <Game/Objects/GameScene/Player/Parry/PlayerParrySystem.h>

// c++
#include <memory>
#include <optional>
#include <unordered_map>
// imgui
#include <imgui.h>

//============================================================================
//	PlayerStateController class
//	プレイヤーの状態管理
//============================================================================
class PlayerStateController :
	public SakuEngine::BaseStateController<PlayerStateConfig> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerStateController() = default;
	~PlayerStateController() = default;

	void Init(Player* player);

	void Update();

	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(BossEnemy* bossEnemy);
	void SetFollowCamera(FollowCamera* followCamera);

	// 状態の強制遷移
	void SetForcedState(PlayerState state);
	// 怯みのリクエスト、遷移可能なら遷移
	void RequestFalterState();

	PlayerState GetCurrentState() const { return GetMachine().GetCurrentId(); }
	PlayerState GetPreviousState() const { return GetMachine().GetPreviousId(); }

	bool IsTriggerParry() const { return inputTransitionPlanner_.GetInputMapper()->IsTriggered(PlayerInputAction::Parry); }
	bool IsActiveParry() const { return parrySystem_.IsActive(); }

	// 今の状態で回避中か
	bool IsAvoidance() { return BaseStateController::GetMachine().GetCurrent().IsAvoidance(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Player* player_;
	BossEnemy* bossEnemy_;

	const std::string kStateJsonPath_ = "Player/stateParameter.json";

	// パリィ処理
	PlayerParrySystem parrySystem_;
	// 入力による状態遷移依頼
	PlayerInputTransitionPlanner inputTransitionPlanner_;
	// 各状態の遷移条件
	std::unordered_map<PlayerState, PlayerStateCondition> conditions_;

	// 各状態の遷移クールタイム
	std::unordered_map<PlayerState, float> lastEnterTime_;
	float currentEnterTime_;

	// 受付済みコンボ
	std::optional<PlayerState> queued_;

	// エディター
	int editingStateIndex_;
	int comboIndex_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// 状態遷移処理
	void DecideExternalTransition() override;
	// 状態が切り替わったときの処理
	void OnStateChanged();

	// helper
	bool Request(PlayerState state);
	bool CanTransition(PlayerState next, bool viaQueue) const;

	// パリィ後の攻撃を行わせるかどうか
	void SetParryAllowAttack(bool allowAttack);

	// privateアクセス許可
	friend class PlayerParrySystem;
	friend class PlayerInputTransitionPlanner;
};
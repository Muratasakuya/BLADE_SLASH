#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/BaseStateController.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Game/Objects/GameScene/Player/State/PlayerStateConfig.h>
#include <Game/Objects/GameScene/Player/Structure/PlayerStructures.h>

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

	void SetBossEnemy(const BossEnemy* bossEnemy);
	void SetFollowCamera(FollowCamera* followCamera);

	// ステータス設定
	void SetStatas(const PlayerStats& stats) { stats_ = stats; }
	// 状態の強制遷移
	void SetForcedState(PlayerState state);
	// 怯みのリクエスト、遷移可能なら遷移
	void RequestFalterState();

	PlayerState GetCurrentState() const { return GetMachine().GetCurrentId(); }
	PlayerState GetPreviousState() const { return previous_; }

	bool IsTriggerParry() const { return inputMapper_->IsTriggered(PlayerInputAction::Parry); }
	bool IsActiveParry() const { return parrySession_.active; }

	// 今の状態で回避中か
	bool IsAvoidance();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// パリィ処理
	struct ParrySession {

		bool active = false;    // 処理中か
		bool reserved = false;  // タイミング待ち
		uint32_t total = 0; // 連続回数
		uint32_t done = 0;  // 処理済み回数
		float reservedStart = 0.0f;

		void Init();
	};

	//--------- variables ----------------------------------------------------

	Player* player_;
	const BossEnemy* bossEnemy_;

	const std::string kStateJsonPath_ = "Player/stateParameter.json";

	// 入力
	std::unique_ptr<SakuEngine::InputMapper<PlayerInputAction>> inputMapper_;
	// ステータス
	PlayerStats stats_;
	// パリィ処理
	ParrySession parrySession_;

	// 各状態の遷移条件
	std::unordered_map<PlayerState, PlayerStateCondition> conditions_;

	// 各状態の遷移クールタイム
	std::unordered_map<PlayerState, float> lastEnterTime_;
	float currentEnterTime_;

	// 前回の状態
	PlayerState previous_;
	// 受付済みコンボ
	std::optional<PlayerState> queued_;

	// 移動入力中のダッシュフラグ
	bool isDashInput_;

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

	// update
	void UpdateInputState();
	void UpdateParryState();
	void RequestParryState();

	// helper
	bool Request(PlayerState state);
	bool CanTransition(PlayerState next, bool viaQueue) const;
	bool IsCombatState(PlayerState state) const;
	bool IsInChain() const;
};
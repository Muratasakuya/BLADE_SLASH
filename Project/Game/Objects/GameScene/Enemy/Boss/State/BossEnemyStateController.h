#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/BaseStateController.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/BossEnemyStateConfig.h>
#include <Game/Objects/GameScene/Enemy/Boss/HUD/BossEnemyAttackSign.h>

// c++
#include <memory>
#include <optional>
#include <functional>
#include <unordered_map>
// imgui
#include <imgui.h>

//============================================================================
//	BossEnemyStateController class
//	ボスの状態を管理する
//============================================================================
class BossEnemyStateController :
	public SakuEngine::BaseStateController<BossEnemyStateConfig> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyStateController() = default;
	~BossEnemyStateController() = default;

	// 各状態の初期化
	void Init(BossEnemy* owner, uint32_t phaseCount);

	// 状態更新
	void Update();

	// エディター
	void ImGui();
	void EditStateTable();

	// 怯み開始
	void StartFalter();

	//--------- accessor -----------------------------------------------------

	// 状態遷移をリクエスト
	void RequestState(BossEnemyState state) { GetMachine().Request(state); }

	void SetPlayer(Player* player);
	void SetFollowCamera(FollowCamera* followCamera);

	void SetDisableTransitions(bool disable) { disableTransitions_ = disable; }

	BossEnemyState GetCurrentState() const { return GetMachine().GetCurrentId(); }
	const ParryParameter& GetParryParam() const { return *parryParam_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	BossEnemy* bossEnemy_;

	// jsonを保存するパス
	const std::string kStateJsonPath_ = "Enemy/Boss/stateParameter.json";
	const std::string kStateTableJsonPath_ = "Enemy/Boss/stateTable.json";

	// 状態デーブル
	BossEnemyStateTable stateTable_;
	// 再生中情報
	uint32_t currentSequenceIndex_;
	uint32_t currentComboSlot_;
	uint32_t currentComboIndex_;
	uint32_t prevPhase_;
	uint32_t prevComboIndex_;
	SakuEngine::StateTimer stateTimer_;

	// 現在のフェーズ
	uint32_t currentPhase_;
	// 状態の強制遷移
	std::optional<BossEnemyState> forcedState_;
	// 攻撃予兆
	std::unique_ptr<BossEnemyAttackSign> attackSign_;
	// 現在のパリィパラメーター
	const ParryParameter* parryParam_;

	// エディター
	BossEnemyState editingState_;
	const ImVec4 kHighlight = ImVec4(1.0f, 0.85f, 0.2f, 1.0f);

	// デバッグ
	// 状態遷移無効フラグ
	bool disableTransitions_;
	// コンボチェックモード
	bool debugComboMode_ = false;         // モードが有効かどうか
	bool debugIgnoreForcedState_ = true;  // 強制遷移を無視するか
	int debugComboIndex_ = -1;            // 固定するコンボID
	float debugNextStateDuration_ = 1.0f; // デバッグ時の遷移間隔

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// コンボリセット
	void ResetCombo();

	// 状態遷移処理
	void DecideExternalTransition() override;
	// 状態が切り替わったときの処理
	void OnStateChanged();

	// update
	// HPに応じたフェーズ更新
	void UpdatePhase();
	// 状態タイマー更新
	void UpdateStateTimer();

	// 次の状態を選択
	void ChooseNextState(const BossEnemyPhase& phase);
	// スタン靭性チェック
	void CheckStunToughness();
	// フェーズ数同期
	void SyncPhaseCount();
	// デバッグコンボの状態設定
	void ChooseNextStateDebug();
	// エディターハイライト描画
	void DrawHighlighted(bool highlight, const ImVec4& col, const std::function<void()>& draw);
};
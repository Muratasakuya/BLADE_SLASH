#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Runtime/Execute/Interface/IPlayerComboExecuteMode.h>

//============================================================================
//	PlayerComboTimelineExecuteMode class
//	プレイヤーのコンボ実行をタイムラインに応じて処理するクラス
//============================================================================
class PlayerComboTimelineExecuteMode :
	public IPlayerComboExecuteMode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboTimelineExecuteMode() = default;
	~PlayerComboTimelineExecuteMode() = default;

	// 初期化
	void Init(Player* player, PlayerComboActionModel* model,
		const PlayerReactionAreaChecker* areaChecker, const SakuEngine::GameObject3D* attackTarget) override;

	// 更新
	void Update() override;

	// コンボ開始
	void StartCombo(uint32_t comboId) override;
	// 外部からのキャンセル要求
	void RequestCancel(bool force) override;

	// 現在時間を直接設定する
	void ScrubToTime(float timeSec, bool pause = true);

	//--------- accessor -----------------------------------------------------

	// 再生、停止の設定
	void SetPlayback(bool playing) { isPlayback_ = playing; }
	// 再生中か
	bool IsPlayback() const { return isPlayback_; }
	// 現在時間
	float GetCurrentTimeSec() const { return currentTime_; }
	// 合計時間
	float GetTotalTime() const { return totalTime_; }

	// モードを取得
	PlayerComboExecuteMode GetMode() const override { return PlayerComboExecuteMode::TimelineDriven; }
	// コンボ実行中か
	bool IsPlaying() const override { return isPlaying_; }
	// 現在再生中のコンボID取得
	uint32_t GetPlayingComboId() const override { return playingComboId_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// コンボステップ実行情報
	struct StepRuntime {

		// コンボ中のステップインデックス
		std::optional<size_t> activeStepIndex;
		// 参照
		PlayerComboActionModel::ComboStep* activeStep = nullptr;
		PlayerComboActionModel::ActionNodeAsset* activeAsset = nullptr;
		PlayerIActionNode* activeNode = nullptr;
	};

	//--------- variables ----------------------------------------------------

	// 処理対象
	Player* player_;
	PlayerComboActionModel* model_;
	const PlayerReactionAreaChecker* areaChecker_;
	const SakuEngine::GameObject3D* attackTarget_;

	// 再生状態
	bool isPlaying_ = false;
	uint32_t playingComboId_ = 0;
	// タイムライン時間を進めるか
	bool isPlayback_ = false;
	// タイムライン時間
	float currentTime_ = 0.0f;
	float totalTime_ = 0.0f;

	// 実行中コンボ
	PlayerComboActionModel::ComboAction* combo_;

	// アクションノードの実行順
	std::vector<size_t> stepOrder_;
	size_t nextStartPos_ = 0;

	// 現在のステップ実行情報
	StepRuntime current_;

	//--------- functions ----------------------------------------------------

	// コンボ停止内部処理
	void StopInternal(bool callExit);

	// コンボの実行順を構築する
	void BuildStepOrder();
	// コンボの合計時間を計算する
	float CalcComboTotalTime() const;

	// ステップの開始、終了処理
	void BeginStep(size_t stepIndexInCombo);
	void EndActiveStep();

	// その時刻に応じて開始すべきステップを開始する
	void StartStepsByTime(float timelineTime);
	// その時刻に応じて状態を再構築する
	void RebuildStateAtTime(float timelineTime);

	// アクティブなステップを時間に応じて更新する
	void UpdateActiveByTime(float timelineTime);
	// ステップの処理時間を取得
	float GetStepDuration(const PlayerComboActionModel::ComboStep& step) const;
	// ノードアセットをIDから探す
	PlayerComboActionModel::ActionNodeAsset* FindAsset(uint32_t nodeAssetId) const;
};
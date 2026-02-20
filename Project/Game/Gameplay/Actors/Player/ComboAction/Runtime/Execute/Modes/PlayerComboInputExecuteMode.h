#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Runtime/Execute/Interface/IPlayerComboExecuteMode.h>

//============================================================================
//	PlayerComboInputExecuteMode class
//	プレイヤーのコンボ実行を入力に応じて処理するクラス
//============================================================================
class PlayerComboInputExecuteMode :
	public IPlayerComboExecuteMode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboInputExecuteMode() = default;
	~PlayerComboInputExecuteMode() = default;

	// 初期化
	void Init(Player* player, PlayerComboActionModel* model,
		const ObjectAreaChecker* areaChecker, const SakuEngine::GameObject3D* attackTarget) override;

	// 更新
	void Update() override;

	// コンボ開始
	void StartCombo(uint32_t comboId) override;
	// 外部からのキャンセル要求
	void RequestCancel(bool force) override;

	//--------- accessor -----------------------------------------------------

	// モードを取得
	PlayerComboExecuteMode GetMode() const override { return PlayerComboExecuteMode::InputDriven; }
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

		// 参照
		PlayerComboActionModel::ComboStep* step = nullptr;
		PlayerComboActionModel::ActionNodeAsset* asset = nullptr;
		PlayerIActionNode* node = nullptr;

		// コンボ中のステップインデックス
		size_t stepIndex = 0;
		uint32_t stepId = 0;

		// ノードの処理合計時間
		float nodeTotalTime = 0.0f;

		// 入力猶予、ステップ内のローカル時間
		float graceStartOffset = 0.0f;
		float graceEndOffset = 0.0f;

		// ノード実行ウィンドウ
		float windowTotalTime = 0.0f;

		// 入力猶予を進捗に変換した値
		float graceStartT = 0.0f;
		float graceEndT = 0.0f;

		// このステップの実行ウィンドウを進めるタイマー
		SakuEngine::StateTimer windowTimer;

		// バッファされた入力があるか
		bool bufferedNext = false;
		// ノードが終了したか
		bool nodeFinished = false;
	};

	//--------- variables ----------------------------------------------------

	// 処理対象
	Player* player_;
	PlayerComboActionModel* model_;
	const ObjectAreaChecker* areaChecker_;
	const SakuEngine::GameObject3D* attackTarget_;

	// 再生状態
	bool isPlaying_ = false;
	uint32_t playingComboId_ = 0;

	// 実行中コンボ
	PlayerComboActionModel::ComboAction* combo_;

	// アクションノードの実行順
	std::vector<size_t> stepOrder_;
	size_t currentOrderPos_ = 0;

	// 現在ステップの実行情報
	StepRuntime current_;

	//--------- functions ----------------------------------------------------

	// ステップ開始処理
	void StartStep(size_t stepIndex);
	// コンボ停止内部処理
	void StopInternal(bool callExit);

	// コンボの実行順を構築する
	void BuildStepOrder();
	// 次のステップが存在するか
	bool HasNextStep() const;

	// 現在のステップインデックス取得
	size_t GetCurrentStepIndex() const { return stepOrder_[currentOrderPos_]; }
	// 次のステップインデックス取得
	size_t GetNextStepIndex() const { return stepOrder_[currentOrderPos_ + 1]; }

	// 入力猶予中にデバイス入力を検知して更新
	void UpdateInputBuffer();
	// アクションノードの更新
	void UpdateNode();
	// 次のステップに進むか、コンボを終了するか判定
	void TryAdvanceOrFinish();

	// 入力判定
	bool IsStepInputTriggered(const PlayerComboActionModel::StepInputSetting& input) const;
	void TryStartComboByStartInput();

	// 参照取得
	PlayerComboActionModel::ActionNodeAsset* FindAsset(uint32_t nodeAssetId) const;
};
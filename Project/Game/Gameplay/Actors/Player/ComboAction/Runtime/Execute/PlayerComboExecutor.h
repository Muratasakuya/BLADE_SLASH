#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Runtime/Execute/Interface/IPlayerComboExecuteMode.h>

//============================================================================
//	PlayerComboExecutor class
//	プレイヤーのコンボ実行を処理するクラス
//============================================================================
class PlayerComboExecutor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboExecutor() = default;
	~PlayerComboExecutor() = default;

	// 初期化
	void Init(Player* player, PlayerComboActionModel* model, const ObjectAreaChecker* areaChecker);

	// 更新
	void Update();

	// コンボ開始
	void StartCombo(uint32_t comboId);
	// 外部によるコンボキャンセル
	void RequestCancel(bool force = false);

	// スクラブ操作に応じた時間設定
	void ScrubTimeline(float timeSec, bool pause = true);

	//--------- accessor -----------------------------------------------------

	// 攻撃対象の設定
	void SetAttackTarget(const SakuEngine::GameObject3D* target);

	// コンボ実行モードをセット
	void SetMode(PlayerComboExecuteMode mode);
	// Executorの有効無効設定
	void SetEnabled(bool enabled);
	// タイムラインで再生制御を行うか設定
	void SetTimelinePlayback(bool playing);

	// コンボ実行モード取得
	PlayerComboExecuteMode GetMode() const { return mode_; }
	// コンボ実行中か
	bool IsPlaying() const;
	// 現在再生中のコンボID取得
	uint32_t GetPlayingComboId() const;
	// Executorが有効か
	bool IsEnabled() const { return isEnabled_; }
	// タイムラインで再生制御を行っているか
	bool IsTimelinePlayback() const;

	// 現在時間、合計時間
	float GetTimelineTime() const;
	float GetTimelineTotalTime() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 処理対象
	Player* player_;
	PlayerComboActionModel* model_;
	const ObjectAreaChecker* areaChecker_;
	const SakuEngine::GameObject3D* attackTarget_;

	// Executorで動かすか
	bool isEnabled_ = false;

	// コンボ実行モード
	PlayerComboExecuteMode mode_ = PlayerComboExecuteMode::InputDriven;

	// 現在の実行モードインスタンス
	std::vector<std::unique_ptr<IPlayerComboExecuteMode>> executeModes_;
	// 現在の実行モード
	IPlayerComboExecuteMode* activeMode_ = nullptr;

	//--------- functions ----------------------------------------------------

	// アクティブモードを再バインド
	void RebindActive();
};
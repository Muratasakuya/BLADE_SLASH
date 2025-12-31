#include "PlayerComboTimelineExecuteMode.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>

//============================================================================
//	PlayerComboTimelineExecuteMode classMethods
//============================================================================

void PlayerComboTimelineExecuteMode::Init(Player* player, PlayerComboActionModel* model,
	const PlayerReactionAreaChecker* areaChecker, const GameObject3D* attackTarget) {

	// 初期化
	player_ = player;
	model_ = model;
	areaChecker_ = areaChecker;
	attackTarget_ = attackTarget;

	// 停止状態にしておく
	StopInternal(false);
}

void PlayerComboTimelineExecuteMode::Update() {

	// 再生中でなければ何もしない
	if (!isPlaying_ || !combo_) {
		return;
	}

	// タイムラインで再生中だけ時間を進める
	if (isPlayback_) {

		currentTime_ += SakuEngine::GameTimer::GetDeltaTime();
		// 負の時間は0.0fに固定
		if (currentTime_ < 0.0f) {
			currentTime_ = 0.0f;
		}
		// 合計時間を超えたら合計時間に固定
		if (totalTime_ < currentTime_) {
			currentTime_ = totalTime_;
		}
	}

	// 時刻に応じて開始、進捗を反映
	StartStepsByTime(currentTime_);
	UpdateActiveByTime(currentTime_);

	// 終了判定、すべてのステップが終了していれば停止させる
	if (totalTime_ <= currentTime_) {
		if (nextStartPos_ >= stepOrder_.size() && !current_.activeStepIndex.has_value()) {
			StopInternal(true);
			return;
		}
	}
}

void PlayerComboTimelineExecuteMode::StartCombo(uint32_t comboId) {

	// 再生中なら停止させて終了させる
	StopInternal(true);

	playingComboId_ = comboId;
	combo_ = model_->FindComboById(comboId);
	// コンボ内のステップが無ければ終了
	if (!combo_ || combo_->steps.empty()) {
		StopInternal(false);
		return;
	}

	// 実行順を作成
	BuildStepOrder();
	// 実行順が無ければ終了
	if (stepOrder_.empty()) {
		StopInternal(false);
		return;
	}

	// 合計時間を計算
	totalTime_ = CalcComboTotalTime();
	if (totalTime_ < 0.1f) {
		totalTime_ = 0.1f;
	}

	// 各アクションノードに処理に必要な情報を設定する
	for (auto& asset : model_->ActionNodes()) {

		asset.implementation->SetPlayer(player_);
		asset.implementation->SetAttackTarget(attackTarget_);
		asset.implementation->SetAreaChecker(areaChecker_);
		asset.implementation->SetIsCancelDisabled(asset.isCancelDisabled);
	}

	// 再生開始
	currentTime_ = 0.0f;
	isPlaying_ = true;
	isPlayback_ = true;
	nextStartPos_ = 0;

	// アクティブ状態をリセット
	current_.activeStepIndex = std::nullopt;
	current_.activeStep = nullptr;
	current_.activeAsset = nullptr;
	current_.activeNode = nullptr;
	// 最初のステップを開始、状態を更新
	StartStepsByTime(0.0f);
	UpdateActiveByTime(0.0f);
}

void PlayerComboTimelineExecuteMode::RequestCancel(bool force) {

	// 再生中でなければ何もしない
	if (!isPlaying_) {
		return;
	}

	// 現在のアクションノードが処理キャンセル不可なら止めない
	if (!force && current_.activeNode && current_.activeNode->IsCancelDisabled()) {
		return;
	}
	// コンボ停止
	StopInternal(true);
}

void PlayerComboTimelineExecuteMode::ScrubToTime(float timeSec, bool pause) {

	// 再生中でなければ何もしない
	if (!isPlaying_ || !combo_) {
		return;
	}

	// ポーズ中は停止させる
	isPlayback_ = !pause;

	// 最小と最大の範囲制限
	currentTime_ = std::clamp(timeSec, 0.0f, totalTime_);

	// 時刻に応じて状態を再構築する
	RebuildStateAtTime(currentTime_);
}

void PlayerComboTimelineExecuteMode::StopInternal(bool callExit) {

	// ノードの終了処理
	if (callExit && current_.activeNode) {
		current_.activeNode->Exit();
	}

	// 再生を停止する
	isPlaying_ = false;
	isPlayback_ = false;
	playingComboId_ = 0;
	combo_ = nullptr;
	// 実行順クリア
	stepOrder_.clear();
	nextStartPos_ = 0;
	// タイムラインリセット
	currentTime_ = 0.0f;
	totalTime_ = 0.0f;
	current_.activeStepIndex = std::nullopt;
	current_.activeStep = nullptr;
	current_.activeAsset = nullptr;
	current_.activeNode = nullptr;
}

void PlayerComboTimelineExecuteMode::BuildStepOrder() {

	// 現在の実行順をクリア
	stepOrder_.clear();
	if (!combo_) {
		return;
	}
	// コンボ内のステップ数分、インデックスを追加
	stepOrder_.reserve(combo_->steps.size());
	for (size_t i = 0; i < combo_->steps.size(); ++i) {
		stepOrder_.push_back(i);
	}

	// 処理の開始順でソートする、同じ開始時間ならステップIDでソートする
	std::sort(stepOrder_.begin(), stepOrder_.end(),
		[&](size_t a, size_t b) {

			const auto& stepA = combo_->steps[a];
			const auto& stepB = combo_->steps[b];
			if (stepA.startTime == stepB.startTime) {
				return stepA.stepId < stepB.stepId;
			}
			return stepA.startTime < stepB.startTime;
		});
}

float PlayerComboTimelineExecuteMode::CalcComboTotalTime() const {

	// コンボが無ければ0.0f
	if (!combo_) {
		return 0.0f;
	}

	// 各ステップの終了時間の最大値を合計時間とする
	float maxEnd = 0.0f;
	for (size_t index : stepOrder_) {

		const auto& step = combo_->steps[index];

		// ノード終了時間
		float nodeDuration = GetStepDuration(step);
		float nodeEnd = step.startTime + nodeDuration;
		// 入力猶予終了時間
		float graceEnd = step.inputGraceStartTime + (std::max)(0.0f, step.inputGraceTime);

		// 大きい方を合計時間候補とする
		float end = (std::max)(nodeEnd, graceEnd);
		if (maxEnd < end) {
			maxEnd = end;
		}
	}
	return maxEnd;
}

float PlayerComboTimelineExecuteMode::GetStepDuration(const PlayerComboActionModel::ComboStep& step) const {

	// ステップ指定のdurationを基本とする
	float duration = step.duration;
	// 負の値は0.0fに補正
	if (duration < 0.0f) {
		duration = 0.0f;
	}

	auto* asset = FindAsset(step.nodeAssetId);
	if (asset && asset->implementation) {

		// ノード実装のGetTotalTime()を取得して設定する
		float t = asset->implementation->GetTotalTime();
		if (0.0f <= t) {
			duration = t;
		}
	}

	// 0.0f対策
	if (duration < Config::kEpsilon) {
		duration = Config::kEpsilon;
	}
	return duration;
}

void PlayerComboTimelineExecuteMode::BeginStep(size_t stepIndexInCombo) {

	// コンボがなければ処理しない
	if (!combo_ || combo_->steps.size() <= stepIndexInCombo) {
		return;
	}

	// 現在処理中のステップがあれば終了させる
	EndActiveStep();

	auto& step = combo_->steps[stepIndexInCombo];
	auto* asset = FindAsset(step.nodeAssetId);
	// ステップ情報を設定
	current_.activeStepIndex = stepIndexInCombo;
	current_.activeStep = &step;
	current_.activeAsset = asset;
	current_.activeNode = asset->implementation.get();

	// これから処理を始めるノードの開始処理
	if (current_.activeNode) {

		current_.activeNode->Enter();
	}
}

void PlayerComboTimelineExecuteMode::EndActiveStep() {

	// ノードの終了処理
	if (current_.activeNode) {
		current_.activeNode->Exit();
	}

	// アクティブ状態をリセット
	current_.activeStepIndex = std::nullopt;
	current_.activeStep = nullptr;
	current_.activeAsset = nullptr;
	current_.activeNode = nullptr;
}

void PlayerComboTimelineExecuteMode::StartStepsByTime(float timelineTime) {

	// コンボが無ければ処理しない
	if (!combo_) {
		return;
	}

	// 開始時間を跨いだステップを開始させる
	while (nextStartPos_ < stepOrder_.size()) {

		// 次に開始すべきステップ情報取得
		size_t stepIndex = stepOrder_[nextStartPos_];
		const auto& step = combo_->steps[stepIndex];
		if (timelineTime + Config::kEpsilon < step.startTime) {
			break;
		}

		// 時間を過ぎたステップを開始
		BeginStep(stepIndex);
		// 次のステップに進める
		++nextStartPos_;
	}
}

void PlayerComboTimelineExecuteMode::RebuildStateAtTime(float timelineTime) {

	// コンボが無ければ処理しない
	if (!combo_) {
		return;
	}

	// 全部終了して作り直す
	EndActiveStep();
	nextStartPos_ = 0;

	// 時刻に応じてステップを開始する
	StartStepsByTime(timelineTime);
	UpdateActiveByTime(timelineTime);
}

void PlayerComboTimelineExecuteMode::UpdateActiveByTime(float timelineTime) {

	// アクティブなステップが無ければ何もしない
	if (!current_.activeStep || !current_.activeStepIndex.has_value()) {
		return;
	}

	// ステップの処理時間取得
	float duration = GetStepDuration(*current_.activeStep);
	// ローカル時間計算
	float localTime = timelineTime - current_.activeStep->startTime;

	// まだ始まってない時間なら何もしない
	if (localTime < 0.0f) {
		return;
	}

	// ローカル時間を進捗に変換
	float progress = localTime / duration;
	progress = std::clamp(progress, 0.0f, 1.0f);

	// タイムライン駆動でノードを更新
	current_.activeNode->SetProgress(progress);

	// 処理時間を超えたらステップ終了
	if (duration <= localTime) {

		EndActiveStep();
	}
}

PlayerComboActionModel::ActionNodeAsset* PlayerComboTimelineExecuteMode::FindAsset(uint32_t nodeAssetId) const {

	if (!model_) {
		return nullptr;
	}
	return model_->FindNodeAssetById(nodeAssetId);
}
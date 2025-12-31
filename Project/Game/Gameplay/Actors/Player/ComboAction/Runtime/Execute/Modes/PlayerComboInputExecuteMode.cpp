#include "PlayerComboInputExecuteMode.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Config.h>

//============================================================================
//	PlayerComboInputExecuteMode classMethods
//============================================================================

void PlayerComboInputExecuteMode::Init(Player* player, PlayerComboActionModel* model,
	const PlayerReactionAreaChecker* areaChecker, const GameObject3D* attackTarget) {

	// 初期化
	player_ = player;
	model_ = model;
	areaChecker_ = areaChecker;
	attackTarget_ = attackTarget;

	// 停止状態にしておく
	StopInternal(false);
}

void PlayerComboInputExecuteMode::Update() {

	// 再生中でなければ何もしない
	if (!isPlaying_) {
		return;
	}

	// ステップ実行ウィンドウ時間更新
	if (current_.windowTotalTime <= Config::kEpsilon) {
		current_.windowTotalTime = Config::kEpsilon;
	}
	current_.windowTimer.Update(current_.windowTotalTime);

	// 入力更新
	UpdateInputBuffer();

	// アクションノード更新
	UpdateNode();

	// 次のアクションノードがあれば進み、なければ終了
	TryAdvanceOrFinish();
}

void PlayerComboInputExecuteMode::UpdateInputBuffer() {

	// 次が無いなら入力受付しない
	if (!HasNextStep() || !current_.step) {
		return;
	}

	// すでに入力がバッファされていれば何もしない
	if (current_.bufferedNext) {
		return;
	}

	// 入力猶予進捗範囲内なら入力を受け付ける
	float t = current_.windowTimer.t_;
	bool inGrace = (current_.graceStartT <= t) && (t <= current_.graceEndT);
	// 猶予時間外なら何もしない
	if (!inGrace) {
		return;
	}
	// 入力があれば予約する
	if (IsStepInputTriggered(current_.step->input)) {
		current_.bufferedNext = true;
	}
}

bool PlayerComboInputExecuteMode::IsStepInputTriggered(
	const PlayerComboActionModel::StepInputSetting& inputSetting) const {

	Input* input = Input::GetInstance();
	bool result = false;

	// 各デバイスで指定の入力があるかチェックする
	if (inputSetting.isUseKeyboard) {

		result |= input->TriggerKey(static_cast<BYTE>(inputSetting.keyDIKCode));
	}
	if (inputSetting.isUseGamePad) {

		result |= input->TriggerGamepadButton(inputSetting.gamePadButton);
	}
	return result;
}

void PlayerComboInputExecuteMode::UpdateNode() {

	// アクションノードがないなら入力猶予待機のみ
	if (!current_.node) {
		current_.nodeFinished = true;
		return;
	}

	// 既に終了しているなら更新しない、入力猶予がまだなら待つ
	if (current_.nodeFinished) {
		return;
	}

	// アクションノード更新
	current_.node->Update();

	// 終了判定
	if (current_.node->GetProgress() <= 1.0f) {

		current_.nodeFinished = true;
	}
}

void PlayerComboInputExecuteMode::TryAdvanceOrFinish() {

	// アクションノードが終了して入力があれば次に進める
	if (current_.nodeFinished && current_.bufferedNext && HasNextStep()) {

		// index更新して次のステップ開始
		++currentOrderPos_;
		StartStep(stepOrder_[currentOrderPos_]);
		return;
	}

	// アクションノードが終了し、入力猶予時間も終了していたらコンボ終了にする
	if (current_.nodeFinished) {

		// 猶予時間終了判定
		bool graceEnded = (current_.graceEndT <= current_.windowTimer.t_) || current_.windowTimer.IsReached();
		// 猶予時間が終了していたらコンボ終了
		if (graceEnded) {

			StopInternal(true);
		}
	}
}

void PlayerComboInputExecuteMode::StartCombo(uint32_t comboId) {

	// 再生中なら停止させて終了させる
	StopInternal(true);

	playingComboId_ = comboId;
	combo_ = model_->FindComboById(comboId);
	// コンボ内のステップが無ければ終了
	if (combo_->steps.empty()) {
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

	// 各アクションノードに処理に必要な情報を設定する
	for (auto& asset : model_->ActionNodes()) {

		asset.implementation->SetPlayer(player_);
		asset.implementation->SetAttackTarget(attackTarget_);
		asset.implementation->SetAreaChecker(areaChecker_);
		asset.implementation->SetIsCancelDisabled(asset.isCancelDisabled);
	}

	// 再生開始
	isPlaying_ = true;
	currentOrderPos_ = 0;
	// 最初のステップ開始
	StartStep(stepOrder_[currentOrderPos_]);
}

void PlayerComboInputExecuteMode::RequestCancel(bool force) {

	// 再生中でなければ何もしない
	if (!isPlaying_) {
		return;
	}

	// 現在のアクションノードが処理キャンセル不可なら止めない
	if (!force && current_.node && current_.node->IsCancelDisabled()) {
		return;
	}
	// コンボ停止
	StopInternal(true);
}

void PlayerComboInputExecuteMode::StartStep(size_t stepIndex) {

	// コンボがなければ処理しない
	if (!combo_ || combo_->steps.size() <= stepIndex) {
		StopInternal(false);
		return;
	}

	// ステップ情報取得
	auto& step = combo_->steps[stepIndex];
	auto* asset = FindAsset(step.nodeAssetId);

	PlayerIActionNode* node = nullptr;
	float nodeTotalTime = step.duration;
	// ノード実装取得、アクションの処理時間取得
	node = asset->implementation.get();
	nodeTotalTime = (std::max)(0.0f, node->GetTotalTime());

	// 入力猶予時間をコンボステップ内のローカル時間に変換
	float graceStartOffset = (std::max)(0.0f, step.inputGraceStartTime - step.startTime);
	float graceEndOffset = graceStartOffset + (std::max)(0.0f, step.inputGraceTime);

	// 入力時間も含めたウィンドウ時間を計算
	float windowTotalTime = (std::max)(nodeTotalTime, graceEndOffset);
	if (windowTotalTime <= Config::kEpsilon) {
		windowTotalTime = Config::kEpsilon;
	}

	// 合計処理時間の中でどの位置で猶予時間が始まるか、終わるかを計算
	// 開始の時間と終了時間を0.0f〜1.0fの範囲に変換
	float graceStartT = std::clamp(graceStartOffset / windowTotalTime, 0.0f, 1.0f);
	float graceEndT = std::clamp(graceEndOffset / windowTotalTime, 0.0f, 1.0f);

	// 処理していたアクションノードの終了処理
	if (current_.node) {

		current_.node->Exit();
	}

	// ランタイム情報設定
	current_ = StepRuntime{};
	current_.step = &step;
	current_.asset = asset;
	current_.node = node;
	// 処理を始めるステップ内のインデックス、IDを設定
	current_.stepIndex = stepIndex;
	current_.stepId = step.stepId;
	// 各ランタイム情報の時間設定
	current_.nodeTotalTime = nodeTotalTime;       // ノード処理合計時間
	current_.graceStartOffset = graceStartOffset; // ノードの開始時間と入力猶予開始時間の差分
	current_.graceEndOffset = graceEndOffset;     // 差分+入力猶予時間
	current_.windowTotalTime = windowTotalTime;   // 当該ステップの実行ウィンドウ時間
	current_.graceStartT = graceStartT;           // 入力猶予開始の進捗時間
	current_.graceEndT = graceEndT;               // 入力猶予終了の進捗時間
	// フラグリセット
	current_.bufferedNext = false;
	current_.nodeFinished = false;
	current_.windowTimer.Reset();

	// これから処理を始めるノードの開始処理
	if (current_.node) {

		current_.node->Enter();
	}
}

void PlayerComboInputExecuteMode::StopInternal(bool callExit) {

	// ノードの終了処理
	if (callExit && current_.node) {
		current_.node->Exit();
	}

	// 再生を停止する
	isPlaying_ = false;
	playingComboId_ = 0;
	combo_ = nullptr;
	// 実行順クリア
	stepOrder_.clear();
	currentOrderPos_ = 0;
	current_ = StepRuntime{};
}

void PlayerComboInputExecuteMode::BuildStepOrder() {

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

bool PlayerComboInputExecuteMode::HasNextStep() const {

	// 次のステップがあるか
	return combo_ != nullptr && (currentOrderPos_ + 1) < stepOrder_.size();
}

PlayerComboActionModel::ActionNodeAsset* PlayerComboInputExecuteMode::FindAsset(uint32_t nodeAssetId) const {

	if (!model_) {
		return nullptr;
	}
	return model_->FindNodeAssetById(nodeAssetId);
}
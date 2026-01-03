#include "PlayerComboExecutor.h"

//============================================================================
//	include
//============================================================================

// modes
#include <Game/Gameplay/Actors/Player/ComboAction/Runtime/Execute/Modes/PlayerComboInputExecuteMode.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Runtime/Execute/Modes/PlayerComboTimelineExecuteMode.h>

//============================================================================
//	PlayerComboExecutor classMethods
//============================================================================

// タイムラインモードを探す
static PlayerComboTimelineExecuteMode* FindTimelineMode(
	const std::vector<std::unique_ptr<IPlayerComboExecuteMode>>& modes) {

	for (const auto& mode : modes) {
		if (mode && mode->GetMode() == PlayerComboExecuteMode::TimelineDriven) {
			return dynamic_cast<PlayerComboTimelineExecuteMode*>(mode.get());
		}
	}
	return nullptr;
}

void PlayerComboExecutor::Init(Player* player,
	PlayerComboActionModel* model, const PlayerReactionAreaChecker* areaChecker) {

	// 初期化
	player_ = player;
	model_ = model;
	areaChecker_ = areaChecker;

	// 実行モードを追加
	executeModes_.emplace_back(std::make_unique<PlayerComboInputExecuteMode>());
	executeModes_.emplace_back(std::make_unique<PlayerComboTimelineExecuteMode>());
}

void PlayerComboExecutor::SetAttackTarget(const SakuEngine::GameObject3D* target) {

	attackTarget_ = target;

	// 入力モードで初期化
	RebindActive();
	if (activeMode_) {

		activeMode_->Init(player_, model_, areaChecker_, attackTarget_);
	}
}

void PlayerComboExecutor::SetEnabled(bool enabled) {

	// 変更がなければ何もしない
	if (isEnabled_ == enabled) {
		return;
	}

	// 有効無効を設定
	isEnabled_ = enabled;

	// 無効化時は強制停止
	if (!isEnabled_ && activeMode_ && activeMode_->IsPlaying()) {
		activeMode_->RequestCancel(true);
	}
}

void PlayerComboExecutor::SetTimelinePlayback(bool playing) {

	auto* timeline = FindTimelineMode(executeModes_);
	if (!timeline) {
		return;
	}

	// Timelineモードがアクティブ状態の時のみ処理させる
	if (mode_ != PlayerComboExecuteMode::TimelineDriven) {
		return;
	}
	timeline->SetPlayback(playing);
}

void PlayerComboExecutor::Update() {

	// すべてのノードの常に行う更新処理
	for (auto& combo : model_->Combos()) {
		for (auto& step : combo.steps) {

			auto* asset = model_->FindNodeAssetById(step.nodeAssetId);
			if (asset && asset->implementation) {
				asset->implementation->UpdateAlways();
			}
		}
	}

	// 現在の実行モードで更新
	if (!activeMode_ || !isEnabled_) {
		return;
	}
	activeMode_->Update();
}

void PlayerComboExecutor::StartCombo(uint32_t comboId) {

	if (!activeMode_) {
		return;
	}
	activeMode_->StartCombo(comboId);
}

void PlayerComboExecutor::RequestCancel(bool force) {

	if (!activeMode_) {
		return;
	}
	activeMode_->RequestCancel(force);
}

void PlayerComboExecutor::ScrubTimeline(float timeSec, bool pause) {

	auto* timeline = FindTimelineMode(executeModes_);
	if (!timeline) {
		return;
	}
	if (mode_ != PlayerComboExecuteMode::TimelineDriven) {
		return;
	}
	timeline->ScrubToTime(timeSec, pause);
}

void PlayerComboExecutor::SetMode(PlayerComboExecuteMode mode) {

	// 変更がなければ何もしない
	if (mode_ == mode) {
		return;
	}

	// 再生中ならコンボを停止させる
	if (activeMode_ && activeMode_->IsPlaying()) {
		activeMode_->RequestCancel(true);
	}

	// モードを変更して再バインド
	mode_ = mode;
	RebindActive();
	// アクティブモードを初期化
	activeMode_->Init(player_, model_, areaChecker_, attackTarget_);
}

bool PlayerComboExecutor::IsPlaying() const {

	return activeMode_ ? activeMode_->IsPlaying() : false;
}

uint32_t PlayerComboExecutor::GetPlayingComboId() const {

	return activeMode_ ? activeMode_->GetPlayingComboId() : 0;
}

bool PlayerComboExecutor::IsTimelinePlayback() const {

	auto* timeline = FindTimelineMode(executeModes_);
	if (!timeline) {
		return false;
	}
	if (mode_ != PlayerComboExecuteMode::TimelineDriven) {
		return false;
	}
	return timeline->IsPlayback();
}

float PlayerComboExecutor::GetTimelineTime() const {

	auto* timeline = FindTimelineMode(executeModes_);
	if (!timeline) {
		return 0.0f;
	}
	if (mode_ != PlayerComboExecuteMode::TimelineDriven) {
		return 0.0f;
	}
	return timeline->GetCurrentTimeSec();
}

float PlayerComboExecutor::GetTimelineTotalTime() const {

	auto* timeline = FindTimelineMode(executeModes_);
	if (!timeline) {
		return 0.0f;
	}
	if (mode_ != PlayerComboExecuteMode::TimelineDriven) {
		return 0.0f;
	}
	return timeline->GetTotalTime();
}

void PlayerComboExecutor::RebindActive() {

	// モードに応じてアクティブモードを切り替え
	for (const auto& mode : executeModes_) {
		if (mode->GetMode() == mode_) {

			activeMode_ = mode.get();
			return;
		}
	}
	// 見つからなかった場合は無効
	activeMode_ = nullptr;
}
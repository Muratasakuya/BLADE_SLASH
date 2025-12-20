#include "BossEnemyStateController.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Engine/Input/Input.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// state
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyIdleState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyTeleportationState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyStunState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyFalterState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyLightAttackState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyJumpAttackState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyStrongAttackState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyChargeAttackState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyRushAttackState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyContinuousAttackState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/BossEnemyProjectileAttackState.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/GreatAttackState/BossEnemyGreatAttackState.h>

//============================================================================
//	BossEnemyStateController classMethods
//============================================================================

namespace {

	// 全てのBossEnemyStateに対して関数を実行
	// Countは除外
	template<class Fn>
	static void ForEachBossEnemyState(Fn&& function) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(BossEnemyState::Count); ++i) {
			BossEnemyState state = static_cast<BossEnemyState>(i);
			if (state == BossEnemyState::Count) {
				continue;
			}
			function(state);
		}
	}
}

void BossEnemyStateController::Init(BossEnemy* owner, uint32_t phaseCount) {

	bossEnemy_ = nullptr;
	bossEnemy_ = owner;

	// 攻撃予兆
	attackSign_ = std::make_unique<BossEnemyAttackSign>();
	attackSign_->Init();

	// 各状態の追加
	auto& machine = BaseStateController::GetMachine();
	machine.Add<BossEnemyIdleState>(BossEnemyState::Idle);
	machine.Add<BossEnemyTeleportationState>(BossEnemyState::Teleport);
	machine.Add<BossEnemyStunState>(BossEnemyState::Stun);
	machine.Add<BossEnemyFalterState>(BossEnemyState::Falter);
	machine.Add<BossEnemyLightAttackState>(BossEnemyState::LightAttack);
	machine.Add<BossEnemyStrongAttackState>(BossEnemyState::StrongAttack);
	machine.Add<BossEnemyChargeAttackState>(BossEnemyState::ChargeAttack);
	machine.Add<BossEnemyRushAttackState>(BossEnemyState::RushAttack);
	machine.Add<BossEnemyContinuousAttackState>(BossEnemyState::ContinuousAttack);
	machine.Add<BossEnemyGreatAttackState>(BossEnemyState::GreatAttack);
	machine.Add<BossEnemyJumpAttackState>(BossEnemyState::JumpAttack);
	machine.Add<BossEnemyProjectileAttackState>(BossEnemyState::ProjectileAttack, phaseCount);

	ForEachBossEnemyState([&](BossEnemyState state) {
		if (!machine.Has(state)) {
			return;
		}
		// 状態遷移対象を設定
		machine.Get(state).SetBossEnemy(owner);
		// 攻撃予兆を設定
		machine.Get(state).SetAttackSign(attackSign_.get());
		// エフェクト生成
		machine.Get(state).CreateEffect();
		});

	// json適応
	ApplyJson();

	// 初期状態を設定
	machine.SetEnter(BossEnemyState::Idle);
	// 現在のパリィ情報を取得
	parryParam_ = &machine.GetCurrent().GetParryParam();
	// 遷移不可フラグ
	disableTransitions_ = false;
}

void BossEnemyStateController::SetPlayer(Player* player) {

	auto& machine = BaseStateController::GetMachine();

	// 各状態にplayerをセット
	ForEachBossEnemyState([&](BossEnemyState state) {
		if (!machine.Has(state)) {
			return;
		}
		machine.Get(state).SetPlayer(player);
		});
}

void BossEnemyStateController::SetFollowCamera(FollowCamera* followCamera) {

	auto& machine = BaseStateController::GetMachine();

	// 各状態にfollowCameraをセット
	ForEachBossEnemyState([&](BossEnemyState state) {
		if (!machine.Has(state)) {
			return;
		}
		machine.Get(state).SetFollowCamera(followCamera);
		if (state == BossEnemyState::GreatAttack) {
			if (auto* greatAttackState = static_cast<BossEnemyGreatAttackState*>(&machine.Get(state))) {

				greatAttackState->InitState();
			}
		}
		});
}

void BossEnemyStateController::StartFalter() {

	auto& machine = GetMachine();

	// Falterへ強制遷移
	machine.Force(BossEnemyState::Falter, true);
	// コンボリセット
	ResetCombo();
}

void BossEnemyStateController::ResetCombo() {

	// 状態遷移の設定をリセット
	currentComboSlot_ = 0;
	currentComboIndex_ = 0;
	prevComboIndex_ = 0;
	currentSequenceIndex_ = 0;
	forcedState_ = std::nullopt;
	stateTimer_.Reset();
}

void BossEnemyStateController::Update() {

	auto& machine = BaseStateController::GetMachine();

	// 遷移不可状態の時、更新のみ行う
	if (disableTransitions_) {

		// 現在の状態を更新
		machine.GetCurrent().Update();

		// 全ての状態の常に行う更新処理
		ForEachBossEnemyState([&](BossEnemyState state) {
			if (machine.Has(state)) {

				machine.Get(state).UpdateAlways();
			}
			});
		return;
	}

	// 状態遷移、更新処理
	BaseStateController::Tick();
	// 状態が変化したかどうかの判定と処理
	OnStateChanged();

	// 全ての状態の常に行う更新処理
	ForEachBossEnemyState([&](BossEnemyState state) {
		if (machine.Has(state)) {

			machine.Get(state).UpdateAlways();
		}
		});

	// 攻撃予兆の更新処理
	attackSign_->Update();
	// 現在のパリィ情報を取得
	parryParam_ = &machine.GetCurrent().GetParryParam();
}

void BossEnemyStateController::DecideExternalTransition() {

	// 状態切り替えの設定処理
	UpdatePhase();
	// スタン靭性チェック
	CheckStunToughness();

	// 状態遷移間隔時間の更新
	UpdateStateTimer();
}

void BossEnemyStateController::OnStateChanged() {

	const auto& machine = BaseStateController::GetMachine();

	BossEnemyState currentState = machine.GetCurrentId();
	// 状態が変化した場合の処理
	if (machine.GetPreviousId() != currentState) {

		bossEnemy_->GetAttackCollision()->SetEnterState(currentState);
	}
}

void BossEnemyStateController::UpdatePhase() {

	const BossEnemyStats& stats = bossEnemy_->GetStats();
	// HPが0なら処理しない
	if (stats.currentHP == 0) {
		return;
	}

	// HP割合に応じて現在のフェーズを計算して設定
	currentPhase_ = bossEnemy_->GetCurrentPhaseIndex();

	// phaseが切り替わったら全てリセットする
	if (prevPhase_ != currentPhase_) {

		prevPhase_ = currentPhase_;
		currentComboIndex_ = 0;
		currentSequenceIndex_ = 0;
		stateTimer_.Reset();

		// 最後のphaseの時
		if (currentPhase_ + 1 == stateTable_.phases.size()) {

			// 強制遷移先を設定
			forcedState_ = BossEnemyState::GreatAttack;
		} else {

			forcedState_.reset();
		}
	}
}

void BossEnemyStateController::CheckStunToughness() {

	// 靭性値が最大になったらスタン状態にする
	const BossEnemyStats& stats = bossEnemy_->GetStats();
	if (stats.currentDestroyToughness == stats.maxDestroyToughness) {

		// 強制遷移先を設定
		forcedState_ = BossEnemyState::Stun;
	}
}

void BossEnemyStateController::UpdateStateTimer() {

	auto& machine = GetMachine();

	// 現在のフェーズの時間を更新
	const auto& phase = stateTable_.phases[currentPhase_];
	// 現在の状態
	BossEnemyIState& state = machine.GetCurrent();
	// 現在の状態ID
	BossEnemyState current = machine.GetCurrentId();

	// 遷移不可
	if (disableTransitions_) {
		stateTimer_.Reset();
		return;
	}

	// 強制遷移中は即座に遷移させる
	// コンボデバッグモード中に無視フラグが立っていると強制遷移しない
	if (forcedState_.has_value() && !(debugComboMode_ && debugIgnoreForcedState_)) {

		// 即座に強制遷移させる
		machine.Force(*forcedState_);
		// コンボリセット
		ResetCombo();
		return;
	}

	// 遷移可能状態になったら時間を進めて遷移させる
	if (state.GetCanExit()) {

		/// 攻撃したかどうか
		const bool isAttack =
			(current == BossEnemyState::LightAttack) ||
			(current == BossEnemyState::StrongAttack) ||
			(current == BossEnemyState::ChargeAttack) ||
			(current == BossEnemyState::RushAttack) ||
			(current == BossEnemyState::GreatAttack) ||
			(current == BossEnemyState::ContinuousAttack);
		// 攻撃状態空の遷移でかつ強制遷移するなら
		if (isAttack && phase.autoIdleAfterAttack) {

			// 待機状態に戻す
			machine.Request(BossEnemyState::Idle);
			// 状態遷移の設定をリセット
			currentComboSlot_ = 0;
			prevComboIndex_ = currentComboIndex_;
			currentSequenceIndex_ = 0;
			stateTimer_.Reset();
			return;
		}

		// 状態遷移の時間を更新
		stateTimer_.Update();
		// 処理時間
		// debugComboMode == true:  デバッグ用の固定時間
		// debugComboMode == false: フェーズごとの設定時間
		const float duration = debugComboMode_ ? debugNextStateDuration_ : phase.nextStateDuration;

		// 遷移時間が過ぎていたら
		if (duration <= stateTimer_.current_) {
			// デバッグ用コンボ固定モードかどうか
			if (debugComboMode_) {
				// 最初のコンボ実行時にコンボを固定する
				if (currentSequenceIndex_ == 0) {

					debugComboIndex_ = std::clamp(debugComboIndex_, 0, static_cast<int>(stateTable_.combos.size() - 1));
					prevComboIndex_ = currentComboIndex_ = debugComboIndex_;
				}
				ChooseNextStateDebug();
			}
			// 通常時の遷移
			else {

				ChooseNextState(phase);
			}

			// 経過時間をリセット
			stateTimer_.Reset();
		}
	}
	// 遷移できない状態
	else {
		stateTimer_.Reset();
	}
}

void BossEnemyStateController::ChooseNextState(const BossEnemyPhase& phase) {

	auto& machine = GetMachine();
	const BossEnemyStats& stats = bossEnemy_->GetStats();

	// 現在のコンボシークエンスが0なら新しいコンボを選択する
	const bool startingNewCombo = currentSequenceIndex_ == 0;
	int maxComboCount = static_cast<int>(stateTable_.combos.size() - 1);
	if (startingNewCombo) {

		// フェーズがコンボを所持していなければ処理しない
		if (phase.comboIndices.empty()) {
			return;
		}

		// 距離レベルを満たすコンボのインデックスフィルター配列
		std::vector<int> filteredComboIndices;
		filteredComboIndices.reserve(phase.comboIndices.size());
		for (int index = 0; index < phase.comboIndices.size(); ++index) {

			int comboIndex = std::clamp(phase.comboIndices[index], 0, maxComboCount);
			const BossEnemyCombo combo = stateTable_.combos[comboIndex];

			// 現在の距離レベルを満たしているかチェック
			auto it = std::ranges::find(combo.requiredDistanceLevels, stats.currentDistanceLevel);
			// 条件を満たしていれば追加
			if (combo.requiredDistanceLevels.empty() || it != combo.requiredDistanceLevels.end()) {

				filteredComboIndices.emplace_back(comboIndex);
			}
		}

		// 該当するコンボがなければすべてを対象にする
		const std::vector<int>& source = filteredComboIndices.empty() ?
			phase.comboIndices : filteredComboIndices;

		// フィルターされたコンボの中からランダムに選択
		const int maxTriCount = 4; // 同じコンボを連続で処理しないように被ったらリトライする最大回数
		int tryCount = 0;          // 試行回数
		do {
			// 1つだけならそのインデックスを渡す
			if (source.size() == 1) {

				currentComboIndex_ = source.front();
			} else {

				// ランダムで選択する
				int randomIndex = SakuEngine::RandomGenerator::Generate(0, static_cast<int>(source.size() - 1));
				currentComboIndex_ = std::clamp(source[randomIndex], 0, maxComboCount);
			}
			++tryCount;
		}
		// 同じコンボを選択したとき試行回数の最大を超えるまでループする
		while (currentComboIndex_ == prevComboIndex_ &&
			!stateTable_.combos[currentComboIndex_].allowRepeat && tryCount < maxTriCount);

		// 現在のコンボスロットを更新
		int currentComboIndex = currentComboIndex_;
		auto it = std::find(phase.comboIndices.begin(), phase.comboIndices.end(), currentComboIndex);
		currentComboSlot_ = (it != phase.comboIndices.end()) ?
			static_cast<uint32_t>(std::distance(phase.comboIndices.begin(), it)) : 0;

		// 前回のコンボインデックスを更新
		prevComboIndex_ = currentComboIndex_;
		currentSequenceIndex_ = 0;
	}

	// 次に再生する状態をコンボのインデックスから取得
	const BossEnemyCombo& combo = stateTable_.combos[currentComboIndex_];
	uint32_t sequenceIndex = currentSequenceIndex_;

	// コンボ内のシークエンスから状態を取得
	BossEnemyState nextState = combo.sequence[sequenceIndex];
	if (!startingNewCombo && !combo.allowRepeat && nextState == machine.GetCurrentId()) {

		// コンボ内のシークエンスが最後まで行ったらリセットする
		sequenceIndex = (sequenceIndex + 1) % combo.sequence.size();
		nextState = combo.sequence[sequenceIndex];
	}

	// シークエンスを進めておく
	currentSequenceIndex_ = sequenceIndex + 1;
	if (combo.sequence.size() <= currentSequenceIndex_) {

		currentSequenceIndex_ = 0;
	}

	// 次の状態を設定
	machine.Request(nextState);
}

void BossEnemyStateController::SyncPhaseCount() {

	const BossEnemyStats& stats = bossEnemy_->GetStats();
	const size_t required = stats.hpThresholds.size() + 1;
	// 足りなければ追加
	while (stateTable_.phases.size() < required) {

		stateTable_.phases.emplace_back(BossEnemyPhase{});
	}
	// 余っていれば削る
	if (stateTable_.phases.size() > required) {

		stateTable_.phases.resize(required);
	}
}

void BossEnemyStateController::ChooseNextStateDebug() {

	if (debugComboIndex_ < 0) {
		return;
	}

	auto& machine = GetMachine();

	// インデックスからコンボを取得
	const BossEnemyCombo& combo = stateTable_.combos[debugComboIndex_];
	if (combo.sequence.empty()) {
		return;
	}

	// 次に再生する状態を取得
	uint32_t sequenceIndex = std::clamp(currentSequenceIndex_, uint32_t(0), static_cast<uint32_t>(combo.sequence.size() - 1));
	BossEnemyState nextState = combo.sequence[sequenceIndex];

	// 連続同一状態の抑制
	if (1 < combo.sequence.size() && !combo.allowRepeat && nextState == machine.GetCurrentId()) {

		// コンボ最後まで行ったらリセット
		sequenceIndex = (sequenceIndex + 1) % combo.sequence.size();
		nextState = combo.sequence[sequenceIndex];
	}

	// indexを次に進める
	currentSequenceIndex_ = sequenceIndex + 1;
	// 範囲を超えたらリセットして次に始めた時に最初からになるようにする
	if (combo.sequence.size() <= currentSequenceIndex_) {

		currentSequenceIndex_ = 0;
	}

	// 次の状態を設定
	GetMachine().Request(nextState);
}

//==============================================================================================================================
//	BossEnemyStateController エディター関連
//==============================================================================================================================

void BossEnemyStateController::DrawHighlighted(bool highlight, const ImVec4& col, const std::function<void()>& draw) {

	if (highlight) {

		ImGui::PushStyleColor(ImGuiCol_Text, col);
	}

	draw();
	if (highlight) {

		ImGui::PopStyleColor();
	}
}

void BossEnemyStateController::ImGui() {

	if (ImGui::Button("SaveJson...stateParameter.json")) {

		SaveJson();
	}

	auto& machine = GetMachine();

	// 各stateの値を調整
	SakuEngine::EnumAdapter<BossEnemyState>::Combo("EditState", &editingState_);
	ImGui::SeparatorText(SakuEngine::EnumAdapter<BossEnemyState>::ToString(editingState_));
	if (machine.Has(editingState_)) {

		machine.Get(editingState_).ImGui();
	}
}

void BossEnemyStateController::EditStateTable() {

	auto& machine = GetMachine();
	const BossEnemyStats& stats = bossEnemy_->GetStats();

	// editorParameters
	const ImVec2 buttonSize = ImVec2(136.0f, 30.0f * 0.64f);

	int currentComboID = -1;
	const auto& curPhaseCombos = stateTable_.phases[currentPhase_].comboIndices;
	if (!curPhaseCombos.empty() && currentComboSlot_ < curPhaseCombos.size()) {
		currentComboID = curPhaseCombos[currentComboSlot_];
	}

	//--------------------------------------------------------------------
	// コンボのデバッグモード
	//--------------------------------------------------------------------

	ImGui::SeparatorText("Debug Combo");
	ImGui::Checkbox("debugComboMode", &debugComboMode_);
	if (debugComboMode_) {

		ImGui::SameLine();
		ImGui::Checkbox("ignoreForcedState", &debugIgnoreForcedState_);
		if (!stateTable_.combos.empty()) {
			if (debugComboIndex_ < 0) {
				debugComboIndex_ = 0;
			}
			ImGui::DragInt("debugComboIndex", &debugComboIndex_, 1, 0, static_cast<int>(stateTable_.combos.size() - 1));
		}
		ImGui::DragFloat("debugNextStateDuration", &debugNextStateDuration_, 0.01f, 0.01f, 10.0f);
	}
	ImGui::Separator();

	//--------------------------------------------------------------------
	// 概要表示
	//--------------------------------------------------------------------

	ImGui::Checkbox("disableTransitions", &disableTransitions_);
	ImGui::Text("currentState: %s", SakuEngine::EnumAdapter<BossEnemyState>::GetEnumName(static_cast<uint32_t>(machine.GetCurrentId())));

	if (ImGui::Button("SaveJson...stateParameter.json")) {

		SaveJson();
	}
	ImGui::Separator();

	//--------------------------------------------------------------------
	// ComboListテーブル
	//--------------------------------------------------------------------

	// 共通フラグ
	const ImGuiTableFlags tableFlags =
		ImGuiTableFlags_BordersInner |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_SizingStretchProp;

	ImGui::SeparatorText("Edit Combo");

	// settings
	const ImVec4 headerColor = ImGui::GetStyleColorVec4(ImGuiCol_Header);
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerColor);

	if (ImGui::Button("CreateCombo")) {
		stateTable_.combos.emplace_back(BossEnemyCombo{});
	}

	if (ImGui::BeginTable("##ComboList", 6, tableFlags)) {

		ImGui::TableSetupColumn("Combo");    // 0
		ImGui::TableSetupColumn("Sequence"); // 1
		ImGui::TableSetupColumn("Rep");      // 2
		ImGui::TableSetupColumn("Teleport"); // 3
		ImGui::TableSetupColumn("Distance"); // 4
		ImGui::TableSetupColumn("AddState"); // 5
		ImGui::TableHeadersRow();

		for (int comboIdx = 0; comboIdx < static_cast<int>(stateTable_.combos.size()); ++comboIdx) {

			auto& combo = stateTable_.combos[comboIdx];
			ImGui::PushID(comboIdx);
			ImGui::TableNextRow();

			//----------------------------------------------------------------
			//  列0: Combo名
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			const std::string comboLabel = "Combo" + std::to_string(comboIdx);
			bool isCurrentCombo = (comboIdx == currentComboID);

			ImGui::AlignTextToFramePadding();
			DrawHighlighted(isCurrentCombo, kHighlight, [&] {
				ImGui::Selectable(("Combo" + std::to_string(comboIdx)).c_str()); });

			if (ImGui::BeginDragDropSource()) {

				ImGui::SetDragDropPayload("ComboIdx", &comboIdx, sizeof(int));
				ImGui::Text("%s", comboLabel.c_str());
				ImGui::EndDragDropSource();
			}

			//----------------------------------------------------------------
			// 列1: Sequence
			//----------------------------------------------------------------
			ImGui::TableNextColumn();
			for (size_t seqIdx = 0; seqIdx < combo.sequence.size();) {

				const int stateId = static_cast<int>(combo.sequence[seqIdx]);
				bool isCurrentState = isCurrentCombo && (combo.sequence[seqIdx] == machine.GetCurrentId());

				ImGui::PushID(static_cast<int>(seqIdx));

				bool clicked = false;
				DrawHighlighted(isCurrentState, kHighlight, [&] {
					clicked = ImGui::Button(SakuEngine::EnumAdapter<BossEnemyState>::GetEnumName(stateId), buttonSize); });
				if (clicked) {

					combo.sequence.erase(combo.sequence.begin() + seqIdx);
					currentSequenceIndex_ = std::min<uint32_t>(currentSequenceIndex_,
						combo.sequence.empty() ? 0u : uint32_t(combo.sequence.size() - 1));

					ImGui::PopID();
					continue;
				} else {
					if (ImGui::BeginDragDropSource()) {

						const int payload = static_cast<int>(seqIdx);
						ImGui::SetDragDropPayload("SeqReorder", &payload, sizeof(int));
						ImGui::Text("%s", SakuEngine::EnumAdapter<BossEnemyState>::GetEnumName(stateId));
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget()) {
						if (auto* payload = ImGui::AcceptDragDropPayload("SeqReorder")) {

							const int fromIdx = *static_cast<const int*>(payload->Data);
							std::swap(combo.sequence[fromIdx], combo.sequence[seqIdx]);
						}
						ImGui::EndDragDropTarget();
					}

					// → 区切り矢印
					ImGui::SameLine();
					if (seqIdx < combo.sequence.size() - 1) {

						ImGui::TextUnformatted("-");
						ImGui::SameLine();
					}

					// 削除しなかったときのみ進める
					++seqIdx;
				}

				ImGui::PopID();
			}

			//----------------------------------------------------------------
			// 列2: Repeat
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			ImGui::Checkbox("##allowRepeat", &combo.allowRepeat);

			//----------------------------------------------------------------
			// 列3: Teleport
			//----------------------------------------------------------------

			ImGui::TableNextColumn();

			ImGui::PushItemWidth(buttonSize.x);
			SakuEngine::EnumAdapter<BossEnemyTeleportType>::Combo("##TeleportType", &combo.teleportType);

			ImGui::PopItemWidth();

			//----------------------------------------------------------------
			// 列4: 距離レベルの追加
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			{
				int id = 0;
				for (const auto& [level, _radius] : stats.distanceLevels) {
					bool checked = (std::ranges::find(combo.requiredDistanceLevels, level)
						!= combo.requiredDistanceLevels.end());
					ImGui::PushID(id++);
					if (ImGui::Checkbox(SakuEngine::EnumAdapter<DistanceLevel>::ToString(level), &checked)) {
						if (checked) {
							combo.requiredDistanceLevels.push_back(level);
						} else {
							if (auto it = std::ranges::find(combo.requiredDistanceLevels, level);
								it != combo.requiredDistanceLevels.end()) {
								combo.requiredDistanceLevels.erase(it);
							}
						}
					}
					ImGui::PopID();
					ImGui::SameLine();
				}
				ImGui::NewLine();
			}

			//----------------------------------------------------------------
			// 列5: AddStateドロップダウン
			//----------------------------------------------------------------
			ImGui::TableNextColumn();
			{
				ImGui::PushItemWidth(buttonSize.x);

				static BossEnemyState selectedState = BossEnemyState::Idle;
				const std::string addLabel = "##" + std::to_string(comboIdx);
				if (SakuEngine::EnumAdapter<BossEnemyState>::Combo(addLabel.c_str(), &selectedState)) {

					const BossEnemyState newState = static_cast<BossEnemyState>(selectedState);
					if (std::ranges::find(combo.sequence, newState) == combo.sequence.end()) {

						combo.sequence.push_back(newState);
					}
				}

				ImGui::PopItemWidth();
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleColor(2);

	ImGui::SeparatorText("Edit Phase");

	//--------------------------------------------------------------------
	// Phasesテーブル
	//--------------------------------------------------------------------

	float duration = stateTable_.phases[currentPhase_].nextStateDuration;
	float progress = std::clamp(stateTimer_.current_ / duration, 0.0f, 1.0f);
	ImGui::ProgressBar(progress, ImVec2(200.0f, 0.0f));

	// 数値表示
	ImGui::SameLine();
	ImGui::Text("%.3f / %.3f", stateTimer_, duration);

	// settings
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerColor);

	// 必要フェーズ数をHP閾値に合わせて調整
	SyncPhaseCount();

	if (ImGui::BeginTable("##Phases", 5, tableFlags)) {

		ImGui::TableSetupColumn("Phase");
		ImGui::TableSetupColumn("Duration");
		ImGui::TableSetupColumn("AutoIdle");
		ImGui::TableSetupColumn("Combos");
		ImGui::TableSetupColumn("AddCombo");
		ImGui::TableHeadersRow();

		for (uint32_t phaseIdx = 0; phaseIdx < static_cast<int>(stateTable_.phases.size()); ++phaseIdx) {

			auto& phase = stateTable_.phases[phaseIdx];
			ImGui::PushID(phaseIdx);
			ImGui::TableNextRow();

			//----------------------------------------------------------------
			// 列0: Phase 名
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			DrawHighlighted(phaseIdx == currentPhase_, kHighlight, [&] {
				ImGui::Text("Phase%d", phaseIdx); });

			//----------------------------------------------------------------
			// 列1: Duration
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			ImGui::DragFloat("##value", &phase.nextStateDuration, 0.01f);

			//----------------------------------------------------------------
			// 列1: AutoIdle
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			ImGui::Checkbox("##autoIdle", &phase.autoIdleAfterAttack);

			//----------------------------------------------------------------
			// 列3: Phaseが保持するCombo
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			for (size_t slotIdx = 0; slotIdx < phase.comboIndices.size();) {

				int comboID = phase.comboIndices[slotIdx];
				bool isCurrentCombo = (phaseIdx == currentPhase_) && (comboID == currentComboID);
				const std::string name = "C:" + std::to_string(comboID);

				ImGui::PushID(static_cast<int>(slotIdx));

				bool clicked = false;
				DrawHighlighted(isCurrentCombo, kHighlight, [&] {
					clicked = ImGui::SmallButton(name.c_str()); });
				if (clicked) {

					phase.comboIndices.erase(phase.comboIndices.begin() + slotIdx);
					if (phaseIdx == currentPhase_) {
						currentComboSlot_ = std::min<uint32_t>(currentComboSlot_,
							phase.comboIndices.empty() ? 0u : uint32_t(phase.comboIndices.size() - 1));
					}
				} else {
					if (ImGui::BeginDragDropSource()) {

						const int payload = static_cast<int>(slotIdx);
						ImGui::SetDragDropPayload("PhaseReorder", &payload, sizeof(int));
						ImGui::Text("%s", name.c_str());
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget()) {
						if (auto* payload = ImGui::AcceptDragDropPayload("PhaseReorder")) {

							const int fromIdx = *static_cast<const int*>(payload->Data);
							std::swap(phase.comboIndices[fromIdx], phase.comboIndices[slotIdx]);
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					++slotIdx;
				}

				ImGui::PopID();
			}

			//----------------------------------------------------------------
			// 列4: AddComboドロップターゲット
			//----------------------------------------------------------------
			ImGui::TableNextColumn();
			ImGui::Dummy(ImVec2(70.0f, 20.0f));

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload("ComboIdx")) {
					const int comboID = *static_cast<const int*>(pl->Data);
					phase.comboIndices.push_back(comboID);
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleColor(2);
}

//==============================================================================================================================
//	BossEnemyStateController Json関連
//==============================================================================================================================

void BossEnemyStateController::ApplyJson() {

	auto& machine = BaseStateController::GetMachine();

	// state
	{
		Json data;
		if (SakuEngine::JsonAdapter::LoadCheck(kStateJsonPath_, data)) {

			// 各状態に対して処理
			ForEachBossEnemyState([&](BossEnemyState state) {

				// キー取得
				const auto& key = SakuEngine::EnumAdapter<BossEnemyState>::ToString(state);
				if (!machine.Has(state) || !data.contains(key)) {
					return;
				}
				machine.Get(state).ApplyJson(data[key]);
				});
		}
	}

	// tabel
	{
		Json data;
		if (SakuEngine::JsonAdapter::LoadCheck(kStateTableJsonPath_, data)) {

			stateTable_.FromJson(data);
		} else {

			stateTable_.combos.clear(); stateTable_.phases.clear();
			// Combo0: Idle
			BossEnemyCombo combo;
			combo.sequence.emplace_back(BossEnemyState::Idle);
			stateTable_.combos.push_back(combo);
			// Phase0がCombo0 を参照
			BossEnemyPhase phase;
			phase.comboIndices.emplace_back(0);
			stateTable_.phases.push_back(phase);
		}
	}
}

void BossEnemyStateController::SaveJson() {

	auto& machine = BaseStateController::GetMachine();

	// state
	{
		Json data;

		// 各状態に対して処理
		ForEachBossEnemyState([&](BossEnemyState state) {
			if (!machine.Has(state)) {
				return;
			}
			machine.Get(state).SaveJson(data[SakuEngine::EnumAdapter<BossEnemyState>::ToString(state)]);
			});

		SakuEngine::JsonAdapter::Save(kStateJsonPath_, data);
	}

	// table
	{
		Json data;
		stateTable_.ToJson(data);

		SakuEngine::JsonAdapter::Save(kStateTableJsonPath_, data);
	}
}

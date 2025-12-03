#include "BossEnemyGreatAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

// state
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/States/BossEnemyGreatAttackCharge.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/States/BossEnemyGreatAttackExecute.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/States/BossEnemyGreatAttackFinish.h>

//============================================================================
//	BossEnemyGreatAttackState classMethods
//============================================================================

BossEnemyGreatAttackState::BossEnemyGreatAttackState() {

	// 各状態を初期化
	states_.emplace(State::Charge, std::make_unique<BossEnemyGreatAttackCharge>());
	states_.emplace(State::Execute, std::make_unique<BossEnemyGreatAttackExecute>());
	states_.emplace(State::Finish, std::make_unique<BossEnemyGreatAttackFinish>());

	// 初期化値
	canExit_ = false;
	editState_ = State::Charge;

	// エフェクト作成
	// フィールドエフェクト
	fieldEffect_ = std::make_unique<EffectGroup>();
	fieldEffect_->Init("fieldEffect", "BossEnemyEffect");
	fieldEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyGreatAttackFieldEffect.json");
	// 雷攻撃
	lightningAttackEffect_ = std::make_unique<EffectGroup>();
	lightningAttackEffect_->Init("lightningAttack", "BossEnemyEffect");
	lightningAttackEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightningAttackEffect.json");
}

void BossEnemyGreatAttackState::InitState(BossEnemy& bossEnemy) {

	for (const auto& state : std::views::values(states_)) {

		state->Init(&bossEnemy, player_, followCamera_, gameLight_);
	}
}

void BossEnemyGreatAttackState::Enter([[maybe_unused]] BossEnemy& bossEnemy) {

	// 初期状態で初期化
	currentState_ = State::Charge;
	states_[currentState_]->Enter();
}

void BossEnemyGreatAttackState::Update([[maybe_unused]] BossEnemy& bossEnemy) {

	// 現在の状態を更新
	auto& state = states_[currentState_];
	state->Update();

	// 処理終了後次の状態に進む
	if (state->CanExit()) {

		// 現在の状態を終了させる
		state->Exit();
		// 次の状態があれば遷移
		if (auto next = GetNextState(currentState_)) {

			currentState_ = *next;
			states_[currentState_]->Enter();

			// 状態別のEnter時のエフェクト発生
			EmitEffect(currentState_);
		} else {

			// 処理終了
			canExit_ = true;
		}
	}
}

void BossEnemyGreatAttackState::UpdateAlways([[maybe_unused]] BossEnemy& bossEnemy) {

	// 常に更新
	for (const auto& state : std::views::values(states_)) {

		state->UpdateAlways();
	}

	// エフェクト更新
	fieldEffect_->Update();
	lightningAttackEffect_->Update();
}

void BossEnemyGreatAttackState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	canExit_ = false;

	// 全てのExitを呼びだして完全にリセット
	for (const auto& state : std::views::values(states_)) {

		state->Exit();
	}
	// エフェクト停止
	fieldEffect_->Stop();
}

void BossEnemyGreatAttackState::ImGui([[maybe_unused]] const BossEnemy& bossEnemy) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));
	EnumAdapter<State>::Combo("State", &editState_);

	ImGui::Separator();
	states_[editState_]->ImGui();
}

void BossEnemyGreatAttackState::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	for (const auto& [state, ptr] : states_) {

		auto key = EnumAdapter<State>::ToString(state);
		ptr->ApplyJson(data[key]);
	}
}

void BossEnemyGreatAttackState::SaveJson(Json& data) {

	for (const auto& [state, ptr] : states_) {

		auto key = EnumAdapter<State>::ToString(state);
		ptr->SaveJson(data[key]);
	}
}

std::optional<BossEnemyGreatAttackState::State>
BossEnemyGreatAttackState::GetNextState(State state) const {

	// 次の遷移状態を返す、無ければnullopt
	switch (state) {
	case State::Charge:  return State::Execute;
	case State::Execute: return State::Finish;
	case State::Finish:  return std::nullopt;
	}
	return std::nullopt;
}

void BossEnemyGreatAttackState::EmitEffect(State state) {

	switch (state) {
	case BossEnemyGreatAttackState::State::Charge:
		break;
	case BossEnemyGreatAttackState::State::Execute:

		// フィールドエフェクト発生
		fieldEffect_->Emit(Vector3::AnyInit(0.0f));
		break;
	case BossEnemyGreatAttackState::State::Finish:
		break;
	}
}
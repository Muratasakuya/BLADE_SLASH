#include "GamecButtonFocusNavigator.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Objects/Base/GameButton.h>

//============================================================================
//	GamecButtonFocusNavigator classMethods
//============================================================================

void GamecButtonFocusNavigator::Init(ButtonFocusGroup group, const std::vector<GameButton*>& items) {

	// 初期化、設定
	group_ = group;
	items_ = items;
	index_ = 0;
	defaultIndex_ = 0;
	hasFocus_ = false;
	axisLatched_ = false;
	ClearFocus();
}

void GamecButtonFocusNavigator::SetGroup(ButtonFocusGroup group,
	const std::vector<GameButton*>& items, size_t defaultIndex) {

	ClearFocus();

	// 初期化、設定
	group_ = group;
	items_ = items;
	defaultIndex_ = (defaultIndex < items_.size()) ? defaultIndex : 0;
	index_ = defaultIndex_;
	hasFocus_ = false;
	axisLatched_ = false;
}

void GamecButtonFocusNavigator::MoveLeft() {

	// indexを左に進める
	if (!items_.empty()) {

		index_ = (index_ + items_.size() - 1) % items_.size();
		ApplyVisuals();
	}
}

void GamecButtonFocusNavigator::MoveRight() {

	// indexを右に進める
	if (!items_.empty()) {

		index_ = (index_ + 1) % items_.size();
		ApplyVisuals();
	}
}

void GamecButtonFocusNavigator::Confirm() {

	if (onConfirm_) {

		onConfirm_(group_, static_cast<int>(index_));
	}
}

void GamecButtonFocusNavigator::ApplyVisuals() {

	if (!hasFocus_) { 
		ClearFocus();
		return;
	}

	for (size_t i = 0; i < items_.size(); ++i) {

		if (!items_[i]) {
			continue;
		}
		// アクティブ状態を設定
		items_[i]->SetFocusActive(i == index_);
	}
}

void GamecButtonFocusNavigator::Update() {

	SakuEngine::Input* input = SakuEngine::Input::GetInstance();

	// マウス(キーボード操作も含む)かパッド操作か判定
	for (const auto& button : items_) {
		if (button) {

			// 判定が競合しないようにする
			button->SetEnableCollision(false);
		}
	}

	// 左右入力判定
	const bool left = input->TriggerGamepadButton(GamePadButtons::ARROW_LEFT);
	const bool right = input->TriggerGamepadButton(GamePadButtons::ARROW_RIGHT);
	const bool decide = input->TriggerGamepadButton(GamePadButtons::A);

	const float lx = input->GetLeftStickVal().x;
	const float dz = input->GetDeadZone();
	if (std::fabs(lx) < dz) axisLatched_ = false;

	const bool axisLeft = (lx <= -dz) && !axisLatched_;
	const bool axisRight = (lx >= dz) && !axisLatched_;

	if (!hasFocus_ && (left || right || decide || axisLeft || axisRight)) {

		index_ = defaultIndex_;
		hasFocus_ = true;
		if (axisLeft || axisRight) {
			axisLatched_ = true;
		}
		ApplyVisuals();
		return;
	}

	// 入力判定結果に応じてインデックスを設定
	if (left) {
		MoveLeft();
	}
	if (right) {

		MoveRight();
	}

	if (axisLeft) {
		MoveLeft();
		axisLatched_ = true;
	}
	if (axisRight) {
		MoveRight();
		axisLatched_ = true;
	}

	if (decide) {

		Confirm();
	}
}

void GamecButtonFocusNavigator::SetFocusIndex(size_t index) {

	// 外部からフォーカスを変更する
	index_ = (index < items_.size()) ? index : 0; ApplyVisuals();
}

void GamecButtonFocusNavigator::ClearFocus() {

	for (const auto& button : items_) {
		if (!button) {

			continue;
		}
		button->SetFocusActive(false);
	}
}
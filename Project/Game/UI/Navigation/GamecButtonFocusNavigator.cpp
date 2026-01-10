#include "GamecButtonFocusNavigator.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/UI/Widgets/Button/GameButton.h>

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

	// --- キーボード入力（A/D） ---
	const bool keyLeft = input->TriggerKey(static_cast<BYTE>(KeyDIKCode::A));
	const bool keyRight = input->TriggerKey(static_cast<BYTE>(KeyDIKCode::D));
	const bool keyDecide =
		input->TriggerKey(static_cast<BYTE>(KeyDIKCode::RETURN)) ||
		input->TriggerKey(static_cast<BYTE>(KeyDIKCode::SPACE));

	// --- ゲームパッド入力（従来） ---
	const bool padDecide = input->TriggerGamepadButton(GamePadButtons::A);

	float lx = input->GetLeftStickVal().x;
	float dz = input->GetDeadZone();
	if (std::fabs(lx) < dz) {
		axisLatched_ = false;
	}

	const bool axisLeft = (lx <= -dz) && !axisLatched_;
	const bool axisRight = (lx >= dz) && !axisLatched_;

	// まとめ
	const bool moveLeft = keyLeft || axisLeft;
	const bool moveRight = keyRight || axisRight;
	const bool decide = keyDecide || padDecide;

	// ★ゲームパッド操作中は常にマウス当たり判定を殺す（従来挙動維持）
	const bool forceNoMouse = (input->GetType() != InputType::Keyboard);

	// 入力がある/フォーカス中/ゲームパッド中ならマウス判定を無効化
	if (forceNoMouse || hasFocus_ || moveLeft || moveRight || decide) {
		for (const auto& button : items_) {
			if (button) button->SetEnableCollision(false);
		}
	}

	// 初回入力でフォーカスを立てる（従来挙動）
	if (!hasFocus_ && (decide || moveLeft || moveRight)) {
		index_ = defaultIndex_;
		hasFocus_ = true;

		if (axisLeft || axisRight) {
			axisLatched_ = true;
		}
		ApplyVisuals();
		return; // ※従来通り「初回はフォーカスだけ」
	}

	// 左右
	if (moveLeft) {
		MoveLeft();
		if (axisLeft) axisLatched_ = true;
	}
	if (moveRight) {
		MoveRight();
		if (axisRight) axisLatched_ = true;
	}

	// 決定
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
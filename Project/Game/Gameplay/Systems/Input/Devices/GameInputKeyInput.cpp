#include "GameInputKeyInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Config.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	GameInputKeyInput classMethods
//============================================================================

GameInputKeyInput::GameInputKeyInput(SakuEngine::Input* input) {

	input_ = input;

	// キーを登録
	keyList_.emplace_back(static_cast<BYTE>(DIK_W));
	keyList_.emplace_back(static_cast<BYTE>(DIK_A));
	keyList_.emplace_back(static_cast<BYTE>(DIK_S));
	keyList_.emplace_back(static_cast<BYTE>(DIK_D));
	keyList_.emplace_back(static_cast<BYTE>(DIK_SPACE));
	keyList_.emplace_back(static_cast<BYTE>(DIK_ESCAPE));
}

float GameInputKeyInput::GetVector(GameInputAction axis) const {

	if (axis == GameInputAction::Use) {

		// マウス移動入力
		SakuEngine::Vector2 value = input_->GetMouseMoveValue();

		// 値が入っていればtrueを返す
		if (value.Length() > Config::kEpsilon) {

			return 1.0f;
		}
	}
	return 0.0f;
}

bool GameInputKeyInput::IsTriggered(GameInputAction button) const {

	if (button == GameInputAction::Use) {

		// スティック入力判定チェック
		float vector = GetVector(button);
		if (vector > Config::kEpsilon) {
			return true;
		}

		// キー入力判定チェック
		for (const auto& key : keyList_) {
			if (input_->TriggerKey(key)) {

				return true;
			}
		}
	}
	return false;
}
#include "GameInputGamePadInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Config.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	GameInputGamePadInput classMethods
//============================================================================

float GameInputGamePadInput::GetVector(GameInputAction axis) const {

	if (axis == GameInputAction::Use) {

		// 左スティック入力
		SakuEngine::Vector2 leftValue = input_->GetLeftStickVal();
		// 右スティック入力
		SakuEngine::Vector2 rightValue = input_->GetRightStickVal();

		// 値が入っていればtrueを返す
		if (leftValue.Length() > Config::kEpsilon ||
			rightValue.Length() > Config::kEpsilon) {

			return 1.0f;
		}
	}
	return 0.0f;
}

bool GameInputGamePadInput::IsTriggered(GameInputAction button) const {

	if (button == GameInputAction::Use) {

		// スティック入力判定チェック
		float vector = GetVector(button);
		if (vector > Config::kEpsilon) {
			return true;
		}

		// ボタン入力判定チェック
		for (size_t index = 0; index <= SakuEngine::EnumAdapter<GamePadButtons>::GetEnumCount(); ++index) {
			if (input_->TriggerGamepadButton(static_cast<GamePadButtons>(index))) {

				return true;
			}
		}
	}
	return false;
}
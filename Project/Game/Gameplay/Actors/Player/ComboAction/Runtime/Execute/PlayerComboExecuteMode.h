#pragma once

//============================================================================
//	PlayerComboExecuteMode enum class
//============================================================================

// プレイヤーのコンボを実行するモード
enum class PlayerComboExecuteMode {

	InputDriven,    // 入力に応じた遷移
	TimelineDriven, // タイムライン時間に応じた遷移
};
#pragma once

//============================================================================
//	include
//============================================================================

//============================================================================
//	InputStructures class
//============================================================================

// XINPUTGamePadのボタンの種類
enum class GamePadButtons {

	ARROW_UP,       // 十字ボタンの上方向
	ARROW_DOWN,     // 十字ボタンの下方向
	ARROW_LEFT,     // 十字ボタンの左方向
	ARROW_RIGHT,    // 十字ボタンの右方向
	START,          // スタートボタン
	BACK,           // バックボタン
	LEFT_THUMB,     // 左スティックのボタン
	RIGHT_THUMB,    // 右スティックのボタン
	LEFT_SHOULDER,  // 左ショルダーボタン（LB）
	RIGHT_SHOULDER, // 右ショルダーボタン（RB）
	LEFT_TRIGGER,   // 左ショルダーボタン（LT）
	RIGHT_TRIGGER,  // 右ショルダーボタン（RT）
	A,              // Aボタン
	B,              // Bボタン
	X,              // Xボタン
	Y,              // Yボタン
	Counts          // ボタンの数を表すための定数
};

// マウス入力の種類
enum class MouseButton {

	Right,
	Left,
	Center
};

// 入力されている状態
enum class InputType {

	Keyboard,
	GamePad,
	Count
};

// 入力検知位置
enum class InputViewArea {

	Game,
	Scene
};
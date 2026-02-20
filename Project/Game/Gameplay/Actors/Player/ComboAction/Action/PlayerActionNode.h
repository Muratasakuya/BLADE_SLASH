#pragma once

//============================================================================
//	PlayerActionNodeType class
//============================================================================

// プレイヤーのアクションノードタイプ
enum class PlayerActionNodeType {

	MoveFrontAttack,   // 前移動攻撃
	MoveWaysAttack,    // 多方向移動攻撃
	ThrowWeaponAttack, // 投擲武器攻撃
	CatchWeaponAttack, // 武器回収攻撃
	ThrustAttack,      // 突き攻撃
	JumpAttack,        // ジャンプ攻撃
};
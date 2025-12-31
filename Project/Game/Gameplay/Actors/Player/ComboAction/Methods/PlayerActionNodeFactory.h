#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Interface/PlayerIActionNode.h>

//============================================================================
//	PlayerActionNodeFactory class
//	プレイヤーアクションノードの生成を行うファクトリークラス
//============================================================================
class PlayerActionNodeFactory {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerActionNodeFactory() = default;
	~PlayerActionNodeFactory() = default;

	// ノード生成
	static std::unique_ptr<PlayerIActionNode> CreateNode(PlayerActionNodeType type);
};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Actors/Player/GuardCondition/PlayerGuardConditionType.h>

// front
class Player;

//============================================================================
//	IPlayerGuardCondition class
//	プレイヤーの状態からtrue/falseを返すインターフェース
//============================================================================
class IPlayerGuardCondition {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPlayerGuardCondition() = default;
	virtual ~IPlayerGuardCondition() = default;

	// エディター
	virtual void ImGui() {}

	// json
	virtual void FromJson([[maybe_unused]] const Json& json) {}
	virtual void ToJson([[maybe_unused]] Json& json) {}

	//--------- accessor -----------------------------------------------------

	// 監視対象の設定
	void SetPlayer(const Player* player) { player_ = player; }

	// 条件結果
	virtual bool GetResult() const = 0;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 監視対象
	const Player* player_;
};
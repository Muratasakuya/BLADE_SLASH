#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/Object/State/StateNode.h>
#include <Engine/MathLib/MathUtils.h>
#include <Engine/Utility/Animation/LerpValue.h>
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/PlayerActionNode.h>

// front
class Player;
class PlayerReactionAreaChecker;

//============================================================================
//	PlayerIActionNode class
//	0.0f~1.0fの間でプレイヤーの動きを細かく分割したアクションノードのインターフェース
//============================================================================
class PlayerIActionNode :
	SakuEngine::StateNode<PlayerActionNodeType> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerIActionNode() = default;
	~PlayerIActionNode() = default;

	// エディター
	virtual void ImGui() {}

	// json
	virtual void FromJson([[maybe_unused]] const Json& data) {}
	virtual void ToJson([[maybe_unused]] Json& data) {}

	//--------- accessor -----------------------------------------------------

	// 処理対象のプレイヤーを設定
	void SetPlayer(Player* player) { player_ = player; }
	// 攻撃対象を設定
	void SetAttackTarget(SakuEngine::GameObject3D* target) { attackTarget_ = target; }
	// リアクションエリアチェッカーを設定
	void SetAreaChecker(const PlayerReactionAreaChecker* checker) { areaChecker_ = checker; }

	// 処理進捗設定
	virtual void SetProgress(float progress) = 0;
	// 処理進捗取得
	virtual float GetProgress() const = 0;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 処理対象
	Player* player_;
	// 攻撃対象
	SakuEngine::GameObject3D* attackTarget_;
	// リアクションエリアチェッカー
	const PlayerReactionAreaChecker* areaChecker_;
};
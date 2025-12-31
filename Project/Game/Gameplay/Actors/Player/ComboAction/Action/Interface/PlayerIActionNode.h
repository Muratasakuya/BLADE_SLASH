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
#include <Game/Gameplay/Actors/Player/Structure/PlayerStructures.h>

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

	// 更新処理
	virtual void Update() override = 0;

	// 状態遷移開始
	virtual void Enter() override {}
	// 状態遷移終了
	virtual void Exit() override {}

	// エディター
	virtual void ImGui() {}

	// json
	virtual void FromJson([[maybe_unused]] const Json& data) {}
	virtual void ToJson([[maybe_unused]] Json& data) {}

	//--------- accessor -----------------------------------------------------

	// 処理対象のプレイヤーを設定
	void SetPlayer(Player* player) { player_ = player; }
	// 攻撃対象を設定
	void SetAttackTarget(const SakuEngine::GameObject3D* target) { attackTarget_ = target; }
	// リアクションエリアチェッカーを設定
	void SetAreaChecker(const PlayerReactionAreaChecker* checker) { areaChecker_ = checker; }

	// キャンセル不可フラグ
	void SetIsCancelDisabled(bool isCancelDisabled) { isCancelDisabled_ = isCancelDisabled; }
	bool IsCancelDisabled() const { return isCancelDisabled_; }

	// 処理進捗設定
	virtual void SetProgress(float progress) = 0;
	// 状態が終了したか
	virtual bool IsFinished() const = 0;
	// 処理合計時間
	virtual float GetTotalTime() const = 0;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 処理対象
	Player* player_;
	// 攻撃対象
	const SakuEngine::GameObject3D* attackTarget_;
	// リアクションエリアチェッカー
	const PlayerReactionAreaChecker* areaChecker_;

	// キャンセル不可フラグ
	bool isCancelDisabled_ = false;
};
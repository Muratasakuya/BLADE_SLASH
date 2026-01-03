#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/KeyframeObject3D.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Interface/PlayerIActionNode.h>

//============================================================================
//	BasePlayerMoveKeyAttack class
//	キーフレームによる移動攻撃の基底クラス
//============================================================================
class BasePlayerMoveKeyAttack :
	public PlayerIActionNode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BasePlayerMoveKeyAttack() = default;
	~BasePlayerMoveKeyAttack() = default;

	// 初期化
	void Init(const std::string& label);

	// トランスフォーム更新
	void UpdateTransform();
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 範囲内に攻撃対象がいるかどうか
	bool isInRange_ = false;

	// 空の親トランスフォーム、攻撃対象が範囲内にいないときに参照する親
	SakuEngine::Transform3D* moveTransform_;
	// タグ
	SakuEngine::ObjectTag* moveTag_;
	// 攻撃対象のトランスフォームの値を補正する用のトランスフォーム
	SakuEngine::Transform3D* fixedTargetTransform_;
	// タグ
	SakuEngine::ObjectTag* fixedTargetTag_;

	// 移動の前座標
	SakuEngine::Vector3 preMovePos_;

	//--------- functions ----------------------------------------------------

	// トランスフォームの作成
	void CreateTransform(const std::string& label, const std::string& name,
		SakuEngine::Transform3D*& transform, SakuEngine::ObjectTag*& tag, bool isCompulsion);

	// 範囲内チェックを行って補間目標を設定する
	void SetTargetByRange(SakuEngine::KeyframeObject3D& keyObject);
};
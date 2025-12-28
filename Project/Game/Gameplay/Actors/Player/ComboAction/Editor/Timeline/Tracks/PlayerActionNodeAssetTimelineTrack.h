#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/Timeline/Interface/IPlayerComboTimelineTrack.h>

//============================================================================
//	PlayerActionNodeAssetTimelineTrack class
//	プレイヤーアクションノードを表示するタイムライントラック
//============================================================================
class PlayerActionNodeAssetTimelineTrack :
	public IPlayerComboTimelineTrack {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerActionNodeAssetTimelineTrack() = default;
	~PlayerActionNodeAssetTimelineTrack() = default;

	void DrawTrack(PlayerComboTimelineDrawContext& context, float trackTopY) override;

	//--------- accessor -----------------------------------------------------

	const char* TrackName() const override { return "ActionNodeAsset"; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 作成するノードタイプ
	PlayerActionNodeType createType_ = PlayerActionNodeType::MoveFrontAttack;

	// クリックした時間、作成時間として渡す
	float createTime_ = 0.0f;
	// 今フレーム移動したか
	bool isMovedThisFrame_ = false;
};
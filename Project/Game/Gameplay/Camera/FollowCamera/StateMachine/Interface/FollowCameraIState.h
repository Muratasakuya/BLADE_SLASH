#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/StateNode.h>
#include <Engine/Object/Base/GameObject3D.h>
#include <Game/Gameplay/Camera/FollowCamera/Structures/FollowCameraStructures.h>

// front
class FollowCamera;

//============================================================================
//	FollowCameraIState class
//	追従カメラの状態インターフェース
//============================================================================
class FollowCameraIState :
	public SakuEngine::StateNode<FollowCameraState> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraIState() = default;
	virtual ~FollowCameraIState() = default;

	// エディター
	virtual void ImGui() = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetFollowCamera(FollowCamera* followCamera) { followCamera_ = followCamera; }
	void SetAnchorObject(const SakuEngine::GameObject3D* anchor) { anchorObject_ = anchor; }
	void SetLookAtTargetObject(const SakuEngine::GameObject3D* lookAtTarget) { lookAtTargetObject_ = lookAtTarget; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 状態遷移対象
	FollowCamera* followCamera_;
	const SakuEngine::GameObject3D* anchorObject_;       // 基準点
	const SakuEngine::GameObject3D* lookAtTargetObject_; // 注視点
};
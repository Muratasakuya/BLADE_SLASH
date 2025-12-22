#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/State/BaseStateController.h>
#include <Game/Gameplay/Camera/FollowCamera/StateMachine/FollowCameraStateConfig.h>

//============================================================================
//	FollowCameraStateMachine class
//	追従カメラの状態、遷移を管理するクラス
//============================================================================
class FollowCameraStateMachine :
	public SakuEngine::BaseStateController<FollowCameraStateConfig> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraStateMachine() = default;
	~FollowCameraStateMachine() = default;

	void Init(FollowCamera* camera);

	void Update();

	void ImGui();

	void UpdateInitialSettings();
	void WarmStartFollow();
	// ブレンド処理が終了しているか
	bool IsFinishedHandoffBlend();

	//--------- accessor -----------------------------------------------------

	void SetAnchorObject(const SakuEngine::GameObject3D* anchor);
	void SetLookAtTargetObject(const SakuEngine::GameObject3D* lookAtTarget);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	FollowCamera* camera_;

	// jsonを保存するパス
	const std::string kStateJsonPath_ = "Camera/Follow/stateParameter.json";

	// エディター
	FollowCameraState editingState_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// 外部遷移判定を行う
	void DecideExternalTransition() override;
};
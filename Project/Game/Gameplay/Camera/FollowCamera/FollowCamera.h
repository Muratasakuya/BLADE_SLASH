#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Game/Gameplay/Actors/Player/Structure/PlayerStructures.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/FollowCameraUpdatePass.h>

//============================================================================
//	FollowCamera class
//	追従カメラ
//============================================================================
class FollowCamera :
	public SakuEngine::BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCamera() = default;
	~FollowCamera() = default;

	void Init() override;
	void LoadAnim();

	void Update() override;

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 処理依存オブジェクトを設定
	void BindDependencies(const FollowCameraDependencies& dependencies);

	// エディターによるカメラアニメーション
	void StartPlayerActionAnim(PlayerState state);
	void StartPlayerActionAnim(const std::string& animName);
	void EndPlayerActionAnim();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 更新パス
	std::unique_ptr<FollowCameraUpdatePass> updatePass_;

	// 状態の管理
	const Player* anchorObject_;          // 基準点
	const BossEnemy* lookAtTargetObject_; // 注視点

	// アニメーションを読み込んだか
	bool isLoadedAnim_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
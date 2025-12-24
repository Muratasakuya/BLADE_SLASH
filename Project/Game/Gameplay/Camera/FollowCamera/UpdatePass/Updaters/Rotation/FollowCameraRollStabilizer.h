#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>

//============================================================================
//	FollowCameraRollStabilizer class
//	カメラのロール回転(Z軸回転)を安定化させるクラス
//============================================================================
class FollowCameraRollStabilizer :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraRollStabilizer() = default;
	~FollowCameraRollStabilizer() = default;

	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::RollStabilizer;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 補間割合速度
	float lerpRate_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
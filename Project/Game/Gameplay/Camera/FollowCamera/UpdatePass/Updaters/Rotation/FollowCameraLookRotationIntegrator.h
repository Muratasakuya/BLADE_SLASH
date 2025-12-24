#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>

//============================================================================
//	FollowCameraLookRotationIntegrator class
//	カメラの注視回転を積分して更新するクラス
//============================================================================
class FollowCameraLookRotationIntegrator :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraLookRotationIntegrator() = default;
	~FollowCameraLookRotationIntegrator() = default;


	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// ヨー回転の取得
	const SakuEngine::Quaternion& GetYawRotation() const { return yawRotation_; }

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::LookRotationIntegrator;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ヨー回転(Y軸回転)
	SakuEngine::Quaternion yawRotation_;
};
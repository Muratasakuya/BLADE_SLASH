#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>

//============================================================================
//	FollowCameraPitchClamper class
//	カメラのピッチ角度(X軸回転)を制限するクラス
//============================================================================
class FollowCameraPitchClamper :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraPitchClamper() = default;
	~FollowCameraPitchClamper() = default;

	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 制限値の取得
	float GetMinPitch() const { return minPitch_; }
	float GetMaxPitch() const { return maxPitch_; }

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::PitchClamper;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 制限角度
	float minPitch_; // 最小
	float maxPitch_; // 最大

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
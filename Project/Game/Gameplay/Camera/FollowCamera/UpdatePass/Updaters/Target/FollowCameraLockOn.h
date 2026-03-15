#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/Direction.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>
#include <Game/Gameplay/Systems/Area/ObjectAreaChecker.h>

//============================================================================
//	FollowCameraLockOn class
//	追従カメラのロックオン処理
//============================================================================
class FollowCameraLockOn :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraLockOn() = default;
	~FollowCameraLockOn() = default;

	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// 処理開始
	void Begin(FollowCameraContext& context) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	void BindDependencies(const FollowCameraDependencies& dependencies) override;

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::LockOn;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// エリアチェッカー
	std::unique_ptr<ObjectAreaChecker> areaChecker_;
	// 目標回転が基準点から見て左か右か
	AnchorToDirection2D lookYawDirection_ = AnchorToDirection2D::Left;

	// 目標X軸回転
	float targetXRotation_ = 0.0f;
	// Y軸オフセット角度
	float lookYawOffset_ = 0.0f;
	// 補間率
	float lookLerpRate_ = 1.0f;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// 目標回転を取得する
	SakuEngine::Quaternion GetTargetRotation() const;

	// ロックオン可能か
	bool IsLockOnAvailable(const FollowCameraFrameService& service) const;
};
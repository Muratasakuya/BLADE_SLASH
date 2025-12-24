#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>

//============================================================================
//	FollowCameraOffsetSmoother class
//	カメラのオフセットを滑らかに補間するクラス
//============================================================================
class FollowCameraOffsetSmoother :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraOffsetSmoother() = default;
	~FollowCameraOffsetSmoother() = default;

	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 現在のオフセット距離を取得
	const SakuEngine::Vector3& GetCurrentOffset() const { return currentOffset_; }

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::OffsetSmoother;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 初期化済みフラグ
	bool initialized_ = false;

	// 現在のオフセット距離
	SakuEngine::Vector3 currentOffset_;

	// 補間割合速度
	SakuEngine::Vector3 lerpRate_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
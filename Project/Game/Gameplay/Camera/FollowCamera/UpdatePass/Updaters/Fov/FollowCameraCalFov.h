#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>
#include <Game/Gameplay/Systems/Area/ObjectAreaChecker.h>

//============================================================================
//	FollowCameraCalFov class
//============================================================================
class FollowCameraCalFov :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraCalFov() = default;
	~FollowCameraCalFov() = default;

	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	void BindDependencies(const FollowCameraDependencies& dependencies) override;

	// 現在の画角を取得
	float GetCalculatedFovY() const { return calculatedFovY_; }

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::CalFov;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 画角の最小値、最大値
	float minFovY_ = 0.64f;
	float maxFovY_ = 0.64f;

	// 計算後の画角
	float calculatedFovY_ = 0.64f;

	// エリアチェッカー
	std::unique_ptr<ObjectAreaChecker> areaChecker_;

	//--------- functions ----------------------------------------------------

	// 補間Tを取得する
	float GetLerpT() const;

	// json
	void ApplyJson();
	void SaveJson();
};
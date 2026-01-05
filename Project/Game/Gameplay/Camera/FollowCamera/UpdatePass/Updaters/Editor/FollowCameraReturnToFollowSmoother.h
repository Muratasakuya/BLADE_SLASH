#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/Easing.h>
#include <Engine/Utility/Timer/StateTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>

//============================================================================
//	FollowCameraReturnToFollowSmoother class
//	エディター更新から追従状態に戻るブレンドを行う
//============================================================================
class FollowCameraReturnToFollowSmoother :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraReturnToFollowSmoother() = default;
	~FollowCameraReturnToFollowSmoother() = default;

	void Init() override;

	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::ToFollowSmoother;
	FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 1フレーム前のエディター更新フラグ
	bool wasInEditorUpdate_ = false;

	// 戻しブレンド中か
	bool isReturning_ = false;
	// 戻しブレンド開始フレーム
	bool justStartedReturn_ = false;

	// 戻しブレンド用タイマー
	SakuEngine::StateTimer returnTimer_;

	// エディター更新終了時の姿勢
	SakuEngine::Vector3 startTranslation_;
	SakuEngine::Quaternion startRotation_ ;
	float startFovY_;

	// エディター更新終了時の追従基準と、そこからのワールドオフセット
	SakuEngine::Vector3 startInterTarget_;
	SakuEngine::Vector3 startWorldOffset_;

	// パラメータ
	float duration_;        // 戻し時間
	EasingType easingType_; // 戻しイージング
	// これ以下なら戻し開始しない閾値
	float startPosThreshold_;
	float startAngleThresholdRad_;
	float startFovThreshold_;
	// X軸回転
	float pitchRotateX_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// エディター更新の終了を検知して、必要ならブレンド開始
	void CheckEndEditorUpdate(FollowCameraContext& context);
	// 戻しブレンド開始
	void StartReturn(FollowCameraContext& context);
	// 戻しブレンド適用
	void ApplyReturnBlend(FollowCameraContext& context);
};

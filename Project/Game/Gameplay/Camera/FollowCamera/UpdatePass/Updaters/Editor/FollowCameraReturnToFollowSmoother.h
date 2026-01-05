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

	// 戻しブレンド中か
	bool IsReturning() const { return isReturning_; }
	// ブレンド係数
	float GetReturnBlendT() const { return returnTimer_.easedT_; }

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
	// 戻しブレンド開始フレーム(開始フレームはt=0で確定させたい)
	bool justStartedReturn_ = false;

	// 戻しブレンド用タイマー
	SakuEngine::StateTimer returnTimer_;

	// エディター更新終了時の姿勢
	SakuEngine::Vector3 startTranslation_{};
	SakuEngine::Quaternion startRotation_ = SakuEngine::Quaternion::Identity();
	float startFovY_ = 0.0f;

	// エディター更新終了時の追従基準(interTarget)と、そこからのワールドオフセット
	SakuEngine::Vector3 startInterTarget_{};
	SakuEngine::Vector3 startWorldOffset_{};

	// パラメータ
	float durationSec_ = 0.35f;                          // 戻し時間(秒)
	EasingType easingType_ = EasingType::EaseOutExpo;    // 戻しイージング
	float startPosThreshold_ = 0.01f;                    // これ以下なら戻し開始しない(位置)
	float startAngleThresholdRad_ = 0.001f;              // これ以下なら戻し開始しない(回転)
	float startFovThreshold_ = 0.001f;                   // これ以下なら戻し開始しない(画角)

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// エディター更新の終了を検知して、必要ならブレンド開始
	void CheckEndEditorUpdate(FollowCameraContext& context, const FollowCameraFrameService& service);

	// 戻しブレンド開始
	void StartReturn(FollowCameraContext& context, const FollowCameraFrameService& service);

	// 戻しブレンド適用
	void ApplyReturnBlend(FollowCameraContext& context);
};

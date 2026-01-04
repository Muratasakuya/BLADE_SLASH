#pragma once

//============================================================================
//	include
//============================================================================
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

	// エディター更新終了直後の姿勢(=現在のカメラ姿勢)を起点として、
	// 各パスが算出した追従姿勢へスムーズに戻す
	void BeginBlendFromCurrentCamera();
	// 任意の姿勢を起点としてブレンド開始
	void BeginBlendFromPose(const SakuEngine::Vector3& translation, const SakuEngine::Quaternion& rotation, float fovY);

	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::ToFollowSmoother;
	FollowCameraUpdatePassID GetID() const override { return ID; }

	//--------- accessor -----------------------------------------------------

	// ブレンド中か
	bool IsBlending() const { return isBlending_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	// json
	void ApplyJson();
	void SaveJson();

	// エディター更新の終了を検知して、必要ならブレンド開始
	void CheckEndEditorUpdate(FollowCameraContext& context);

	// half-life(半減期)からフレーム補間率へ
	float HalfLifeToAlpha(float halfLife, float deltaTime);

	//========================================================================
	//	variables
	//========================================================================

	// ブレンド中か
	bool isBlending_ = false;
	// ブレンド経過時間
	float blendTimer_ = 0.0f;

	// ブレンドするカメラ姿勢
	SakuEngine::Vector3 blendedPos_{};
	SakuEngine::Quaternion blendedRotate_ = SakuEngine::Quaternion::Identity();
	float blendedFovY_ = 0.0f;

	// ブレンドパラメータ
	// half-life: 値が目標との差分を半分にするまでの時間(秒)
	float posHalfLife_ = 0.15f;
	float rotateHalfLife_ = 0.15f;
	float fovHalfLife_ = 0.15f;
	// 最大ブレンド時間(秒)
	float maxBlendTime_ = 2.0f;

	float pitchRotateX_ = 0.0f;

	// 終了判定閾値
	float finishPosEpsilon_ = 0.01f;
	float finishRotateEpsilonDeg_ = 0.25f;
	float finishFovEpsilon_ = 0.01f;

	// 1フレーム前のエディター更新フラグ
	bool wasInEditorUpdate_ = false;
};

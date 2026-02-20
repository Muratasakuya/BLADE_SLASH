#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/InputMapper.h>
#include <Engine/MathLib/Vector2.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>
#include <Game/Gameplay/Camera/FollowCamera/Input/Actions/FollowCameraInputAction.h>

//============================================================================
//	FollowCameraLookInputSmoother class
//	カメラの視点入力を処理し、滑らかに補間処理するクラス
//============================================================================
class FollowCameraLookInputSmoother :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraLookInputSmoother() = default;
	~FollowCameraLookInputSmoother() = default;

	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 1フレームの入力による回転量を取得
	const SakuEngine::Vector2& GetFrameRotationDelta() const { return frameRotationDelta_; }

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::LookInputSmoother;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 入力クラス
	std::unique_ptr<SakuEngine::InputMapper<FollowCameraInputAction>> inputMapper_;

	// 入力の値補間用
	SakuEngine::Vector2 smoothedInput_;
	// 入力補間割合速度
	float inputLerpRate_;

	// 各入力の感度
	SakuEngine::Vector2 padSensitivity_;   // ゲームパッド
	SakuEngine::Vector2 mouseSensitivity_; // マウス

	// 1フレームの入力による回転量
	SakuEngine::Vector2 frameRotationDelta_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
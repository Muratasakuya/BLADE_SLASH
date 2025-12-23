#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/InputMapper.h>
#include <Engine/MathLib/Vector2.h>
#include <Game/Gameplay/Camera/FollowCamera/Input/Actions/FollowCameraInputAction.h>

//============================================================================
//	FollowCameraLookInputSmoother class
//	カメラの視点入力を処理し、滑らかに補間処理するクラス
//============================================================================
class FollowCameraLookInputSmoother {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraLookInputSmoother() = default;
	~FollowCameraLookInputSmoother() = default;

	// 初期化
	void Init();

	// 更新
	void Update(float deltaTime);

	// エディター
	void ImGui();

	// 現在の入力補間値をリセットする
	void ResetSmoothedInput() { smoothedInput_ = SakuEngine::Vector2::AnyInit(0.0f); }

	//--------- accessor -----------------------------------------------------

	// 1フレームの入力による回転量を取得
	const SakuEngine::Vector2& GetFrameRotationDelta() const { return frameRotationDelta_; }
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
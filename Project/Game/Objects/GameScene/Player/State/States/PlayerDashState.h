#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>

//============================================================================
//	PlayerDashState class
//	ダッシュ状態
//============================================================================
class PlayerDashState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerDashState(const SakuEngine::InputMapper<PlayerInputAction>* inputMapper);
	~PlayerDashState() = default;

	void Enter() override;

	void Update() override;

	void Exit() override;

	// imgui
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	bool GetCanExit() const override;
	bool IsAvoidance() const override { return isAvoiding_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	enum class State {

		Accel,   // 加速
		Sustain, // 待ち
		Decel    // 減速
	};

	//--------- variables ----------------------------------------------------

	// 入力
	const SakuEngine::InputMapper<PlayerInputAction>* inputMapper_;

	State currentState_;

	SakuEngine::Vector3 move_; // 移動量
	float moveSpeed_; // 移動速度
	float sustainTimer_; // 最高速を維持する時間経過
	float sustainTime_;  // 最高速を維持する時間

	// ダッシュの速度補間
	std::unique_ptr<SakuEngine::SimpleAnimation<float>> accelLerp_;
	std::unique_ptr<SakuEngine::SimpleAnimation<float>> decelLerp_;

	// 回避中か
	bool isAvoiding_ = false;

	//--------- functions ----------------------------------------------------

	void UpdateState();
	void UpdateDash();
};
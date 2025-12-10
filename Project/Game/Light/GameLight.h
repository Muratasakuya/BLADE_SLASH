#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Light/PunctualLight.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	GameLight class
//============================================================================
class GameLight :
	public BasePunctualLight {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameLight() = default;
	~GameLight() = default;

	void DerivedInit() override;

	void Update() override;

	void DerivedImGui() override;

	// 補間開始
	void Start();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		None,          // 何もしない
		BeginUpdating, // 更新入り
		Wait,          // 最初の更新状態で待機
		EndUpdating    // 元に戻す
	};

	template <typename T>
	struct LerpValue {

		T start; // 開始値
		T end;   // 目標値
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 補間時間
	StateTimer beginUpdateTimer_; // 補間開始タイマー
	StateTimer endUpdateTimer_;   // 補間終了タイマー

	// 補間する値(スポットライト)
	LerpValue<Color> color_;
	LerpValue<Vector3> pos_;
	LerpValue<float> intensity_;
	LerpValue<float> distance_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
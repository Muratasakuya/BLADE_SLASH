#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/StateMachine/Interface/PlayerIState.h>
#include <Engine/Editor/Effect/User/EffectGroup.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	PlayerParryState class
//	パリィ状態、入力に応じてカウンター攻撃を行う
//============================================================================
class PlayerParryState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerParryState(const SakuEngine::InputMapper<PlayerInputAction>* inputMapper);
	~PlayerParryState() = default;

	void Enter() override;

	void Update() override;
	void UpdateAlways() override;

	void Exit() override;

	// imgui
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetAllowAttack(bool allow) { allowAttack_ = allow; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class RequestState {

		PlayAnimation,  // animationの再生を行う
		AttackAnimation // animationの再生中の処理
	};

	struct LerpParameter {

		float timer; // 補間時間
		float time;  // 補間にかける時間
		EasingType easingType;
		float moveDistance; // 移動距離
		bool isFinished;     // 座標補間が終了したか
	};

	//--------- variables ----------------------------------------------------

	// 入力
	const SakuEngine::InputMapper<PlayerInputAction>* inputMapper_;

	// 攻撃制御
	bool allowAttack_;
	bool isEmittedBlur_;

	// parameters
	LerpParameter parryLerp_;
	LerpParameter attackLerp_;

	SakuEngine::Vector3 startPos_;  // 開始座標
	SakuEngine::Vector3 targetPos_; // 目標座標

	float deltaWaitTimer_; // deltaTimeが元に戻るまでの時間経過
	float deltaWaitTime_;  // deltaTimeが元に戻るまでの時間
	float deltaLerpSpeed_; // 補間速度
	float cameraLookRate_; // カメラ補間速度

	std::optional<RequestState> request_;

	// パリィエフェクト
	// ヒットした瞬間
	std::unique_ptr<SakuEngine::EffectGroup> parryHitEffect_;
	float parryHitEffectPosY_;
	// 引きずる剣先
	std::unique_ptr<SakuEngine::EffectGroup> tipScrackEffect_;
	// 攻撃ヒットエフェクト
	std::unique_ptr<SakuEngine::EffectGroup> hitEffect_;
	float hitEffectOffsetY_;

	// サウンド
	// パリィヒット音
	const std::string parryHitSE_ = "parryHitSE_1";
	// 実際にならす音量
	float parryHitSEVolume_ = 1.0f;
	// パリィヒット音の基本音量
	float parryHitSEBaseVolume_ = 1.0f;
	// パリィ回数に応じた音量の加算値
	float parryHitSEAddVolume_ = 0.1f;

	//--------- functions ----------------------------------------------------

	void UpdateDeltaWaitTime();
	void UpdateLerpTranslation();
	void CheckInput();
	void UpdateAnimation();

	// helper
	SakuEngine::Vector3 GetLerpTranslation(LerpParameter& lerp);
	SakuEngine::Vector3 SetLerpValue(SakuEngine::Vector3& start, SakuEngine::Vector3& target,
		float moveDistance, bool isPlayerBase);
};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2DArray.h>
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Game/UI/Widgets/HPBar/GameHPBar.h>
#include <Game/UI/Widgets/DigitDisplay/GameDigitDisplay.h>
#include <Game/UI/Widgets/DamageDisplay/GameDisplayDamage.h>
#include <Game/UI/Structures/GameCommonStructures.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Structures/BossEnemyStructures.h>

// c++
#include <deque>
// front
class BossEnemy;

//============================================================================
//	BossEnemyHUD class
//	ボスの情報を表示するHUD
//============================================================================
class BossEnemyHUD {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyHUD() = default;
	~BossEnemyHUD() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(const BossEnemy* bossEnemy) { bossEnemy_ = bossEnemy; }
	void SetFollowCamera(const SakuEngine::BaseCamera* camera) { camera_ = camera; }
	void SetDamage(int damage);
	void SetDisable();
	void SetValid();

	bool IsDisable() const { return isDisable_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const BossEnemy* bossEnemy_;
	const SakuEngine::BaseCamera* camera_;

	// HP背景
	std::unique_ptr<SakuEngine::GameObject2D> hpBackground_;
	GameCommon::HUDInitParameter hpBackgroundParameter_;
	// HP残量
	std::unique_ptr<GameHPBar> hpBar_;
	GameCommon::HUDInitParameter hpBarParameter_;
	// 撃破靭性値
	std::unique_ptr<GameHPBar> destroyBar_;
	GameCommon::HUDInitParameter destroyBarParameter_;
	// 撃破靭性値の数字表示
	std::unique_ptr<GameDigitDisplay> destroyNumDisplay_;
	GameCommon::HUDInitParameter destroyNumParameter_;
	SakuEngine::Vector2 destroyNumOffset_; // オフセット座標
	SakuEngine::Vector2 destroyNumSize_;   // サイズ
	// 名前文字表示
	std::unique_ptr<SakuEngine::GameObject2D> nameText_;
	GameCommon::HUDInitParameter nameTextParameter_;
	// ダメージ表示
	std::unique_ptr<GameDisplayDamage> damageDisplay_;

	// フェーズ閾値表示
	std::unique_ptr<SakuEngine::GameObject2DArray> phaseThreshold_;
	SakuEngine::Color enablePhaseThresholdColor_;  // 有効時の色
	SakuEngine::Color disablePhaseThresholdColor_; // 無効時の色

	// parameters
	float returnAlphaTimer_; // alpha値を元に戻すときの経過時間
	float returnAlphaTime_;  // alpha値を元に戻すときの時間
	EasingType returnAlphaEasingType_;

	bool isDisable_;   // 無効状態かどうか
	bool returnVaild_; // 再度有効にする

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitSprite();

	// update
	void UpdateSprite();
	void UpdateAlpha();

	// ボスのフェーズ閾値表示位置の更新
	void UpdatePhaseThresholdPos();
};
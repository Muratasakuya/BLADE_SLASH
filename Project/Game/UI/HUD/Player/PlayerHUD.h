#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/InputStructures.h>
#include <Engine/Utility/Material/SpriteVertexColorAnimation.h>
#include <Engine/Object/Base/GameObject2DArray.h>
#include <Game/UI/Widgets/HPBar/GameHPBar.h>
#include <Game/UI/Widgets/DigitDisplay/GameDigitDisplay.h>
#include <Game/UI/Widgets/DamageDisplay/GameDisplayDamage.h>
#include <Game/UI/Structures/GameCommonStructures.h>
#include <Game/Gameplay/Actors/Player/Structure/PlayerStructures.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Structures/BossEnemyStructures.h>

// c++
#include <utility>
// front
class Player;
class BossEnemy;
class FollowCamera;

//============================================================================
//	PlayerHUD class
//	プレイヤーの情報を表示するHUD
//============================================================================
class PlayerHUD {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerHUD() = default;
	~PlayerHUD() = default;

	// 初期化
	void Init();

	// HUD表示の更新
	void Update(const Player& player);

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	// 入力示唆アニメーション開始、終了呼び出し
	void StartInputSuggest();
	void EndInputSuggest();

	void SetStatas(const PlayerStats& stats) { stats_ = stats; }
	void SetDamage(int damage);
	void SetFollowCamera(const FollowCamera* followCamera) { followCamera_ = followCamera; }
	void SetBossEnemy(const BossEnemy* bossEnemy) { bossEnemy_ = bossEnemy; }
	void SetDisable();
	void SetValid();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 入力示唆
	struct Suggest {

		// 表示スプライト
		std::unique_ptr<SakuEngine::GameObject2D> sprite;

		// アニメーション
		SakuEngine::SimpleAnimation<SakuEngine::Vector2> sizeAnim;
		SakuEngine::SimpleAnimation<SakuEngine::Color> colorAnim;
		SakuEngine::SimpleAnimation<float> emissiveAnim;
	};

	//--------- variables ----------------------------------------------------

	const FollowCamera* followCamera_;
	const BossEnemy* bossEnemy_;

	// ステータス
	PlayerStats stats_;
	// 現在の入力状態
	InputType inputType_;
	InputType preInputType_;

	//----------- stats ------------------------//

	// HP背景
	std::unique_ptr<SakuEngine::GameObject2D> hpBackground_;
	GameCommon::HUDInitParameter hpBackgroundParameter_;
	// HP残量
	std::unique_ptr<GameHPBar> hpBar_;
	GameCommon::HUDInitParameter hpBarParameter_;
	SakuEngine::SpriteVertexColorAnimation hpBarColorAnim_;
	// スキル値
	std::unique_ptr<GameHPBar> skilBar_;
	GameCommon::HUDInitParameter skilBarParameter_;
	SakuEngine::SpriteVertexColorAnimation skilBarColorAnim_;

	// 名前文字表示
	std::unique_ptr<SakuEngine::GameObject2D> nameText_;
	GameCommon::HUDInitParameter nameTextParameter_;

	// ダメージ表示
	std::unique_ptr<GameDisplayDamage> damageDisplay_;

	// ボタン入力示唆
	static const uint32_t kInputSuggestCount_ = 2;
	std::array<Suggest, kInputSuggestCount_> inputSuggests_;

	// スキルP閾値表示
	std::unique_ptr<SakuEngine::GameObject2DArray> skillThreshold_;
	SakuEngine::Color enableSkillThresholdColor_;  // 有効時の色
	SakuEngine::Color disableSkillThresholdColor_; // 無効時の色

	//----------- operate ----------------------//

	// 入力アイコン表示
	std::unique_ptr<SakuEngine::GameObject2DArray> operateIcons_;

	float returnAlphaTimer_; // alpha値を元に戻すときの経過時間
	float returnAlphaTime_;  // alpha値を元に戻すときの時間
	EasingType returnAlphaEasingType_;
	// 入力示唆
	bool isInputSuggestActive_;    // 入力示唆アニメーションが有効かどうか
	bool endDelayInputSuggest_;    // 遅延時間が終わったかどうか
	SakuEngine::StateTimer inputSuggestDelay_; // 表示遅延時間、初期発生時
	SakuEngine::Vector3 inputSuggestEmissionColor_; // 入力示唆の発光色

	bool isDisable_;   // 無効状態かどうか
	bool returnVaild_; // 再度有効にする

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitSprite();

	// update
	void UpdateSprite(const Player& player);
	void UpdateAlpha();

	// バー更新
	void UpdateBar();
	// 入力示唆更新
	void UpdateInputSuggest();
	// スキルP閾値表示更新
	void UpdateSkillThreshold();

	// helper
	void ChangeAllOperateSprite();
};
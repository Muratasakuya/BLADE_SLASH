#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/InputStructures.h>
#include <Game/Objects/Base/GameHPBar.h>
#include <Game/Objects/Base/GameDisplayDamage.h>
#include <Game/Objects/Base/GameCommonStructures.h>
#include <Game/Objects/GameScene/Player/Structure/PlayerStructures.h>
#include <Game/Objects/GameScene/Enemy/Boss/Structures/BossEnemyStructures.h>

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
	// ボス敵のパリィ受付状態の確認
	void CheckBossEnemyParry();

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

	// 入力状態に応じて変化するsprite
	struct InputStateSprite {

		std::unique_ptr<GameObject2D> staticSprite;
		std::unordered_map<InputType, std::unique_ptr<GameObject2D>> dynamicSprites;

		uint32_t index; // spriteを左から並べた時の順番

		// groupの名前
		const std::string groupName = "PlayerHUD";

		void Init(uint32_t spriteIndex, const std::string& staticSpriteTextureName,
			const std::unordered_map<InputType, std::string>& dynamicSpritesTextureName);

		void ChangeDynamicSprite(InputType type);

		void SetTranslation(const Vector2& leftSpriteTranslation,
			float dynamicSpriteOffsetY, float operateSpriteSpancingX);

		void SetSize(const Vector2& staticSpriteSize,
			const Vector2& dynamicSpriteSize_);

		void SetAlpha(InputType type, float alpha);
	};

	// 入力示唆
	struct Suggest {

		// 表示スプライト
		std::unique_ptr<GameObject2D> sprite;

		// アニメーション
		SimpleAnimation<Vector2> sizeAnim;
		SimpleAnimation<Color> colorAnim;
		SimpleAnimation<float> emissiveAnim;
	};

	//--------- variables ----------------------------------------------------

	const FollowCamera* followCamera_;
	const BossEnemy* bossEnemy_;

	// ボスの状態を監視してパリィ入力示唆を出す
	std::optional<BossEnemyState> exitParryBossEnemyState_;
	bool isCanParryBossEnemy_;

	// ステータス
	PlayerStats stats_;
	// 現在の入力状態
	InputType inputType_;
	InputType preInputType_;

	//----------- stats ------------------------//

	// HP背景
	std::unique_ptr<GameObject2D> hpBackground_;
	GameCommon::HUDInitParameter hpBackgroundParameter_;
	// HP残量
	std::unique_ptr<GameHPBar> hpBar_;
	GameCommon::HUDInitParameter hpBarParameter_;
	// スキル値
	std::unique_ptr<GameHPBar> skilBar_;
	GameCommon::HUDInitParameter skilBarParameter_;

	// 名前文字表示
	std::unique_ptr<GameObject2D> nameText_;
	GameCommon::HUDInitParameter nameTextParameter_;

	// ダメージ表示
	std::unique_ptr<GameDisplayDamage> damageDisplay_;

	// ボタン入力示唆
	static const uint32_t kInputSuggestCount_ = 2;
	std::array<Suggest, kInputSuggestCount_> inputSuggests_;

	//----------- operate ----------------------//

	// 操作方法表示
	InputStateSprite attack_; // 攻撃
	InputStateSprite dash_;   // ダッシュ/回避
	InputStateSprite skil_;   // スキル
	InputStateSprite parry_;  // パリィ

	// parameters
	Vector2 leftSpriteTranslation_; // 左端のsprite座標
	float dynamicSpriteOffsetY_;    // 入力状態に応じて変化するspriteのオフセットY座標
	float operateSpriteSpancingX_;  // indexに応じてオフセットをかける
	// それぞれのspriteのサイズ
	Vector2 staticSpriteSize_;
	Vector2 dynamicSpriteSize_;
	float returnAlphaTimer_; // alpha値を元に戻すときの経過時間
	float returnAlphaTime_;  // alpha値を元に戻すときの時間
	EasingType returnAlphaEasingType_;
	// 入力示唆
	bool isInputSuggestActive_;    // 入力示唆アニメーションが有効かどうか
	bool endDelayInputSuggest_;    // 遅延時間が終わったかどうか
	StateTimer inputSuggestDelay_; // 表示遅延時間、初期発生時
	Vector3 inputSuggestEmissionColor_; // 入力示唆の発光色

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

	// 入力示唆更新
	void UpdateInputSuggest();

	// helper
	void ChangeAllOperateSprite();
	void SetAllOperateTranslation();
	void SetAllOperateSize();
};
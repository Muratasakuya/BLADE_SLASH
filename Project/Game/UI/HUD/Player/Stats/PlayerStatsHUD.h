#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Material/SpriteVertexColorAnimation.h>
#include <Engine/Object/Base/GameObject2DArray.h>
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Game/UI/Widgets/HPBar/GameHPBar.h>
#include <Game/UI/Widgets/DamageDisplay/GameDisplayDamage.h>
#include <Game/UI/Structures/GameCommonStructures.h>

// front
class PlayerShareHUD;
class Player;

//============================================================================
//	PlayerStatsHUD class
//	プレイヤーのステータス表示HUD
//============================================================================
class PlayerStatsHUD {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerStatsHUD() = default;
	~PlayerStatsHUD() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetPlayer(const Player* player) { player_ = player; }
	void SetCamera(const SakuEngine::BaseCamera* camera) { camera_ = camera; }
	void SetShareHUD(const PlayerShareHUD* shareHud) { shareHud_ = shareHud; }

	// ダメージ表示設定
	void SetDamage(int damage);

	// 表示、非表示設定
	void SetIsDisplay(bool isDisplay);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const Player* player_;
	const SakuEngine::BaseCamera* camera_;
	const PlayerShareHUD* shareHud_;

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

	// 名前文字表示
	std::unique_ptr<SakuEngine::GameObject2D> nameText_;
	GameCommon::HUDInitParameter nameTextParameter_;

	// ダメージ表示
	std::unique_ptr<GameDisplayDamage> damageDisplay_;

	// スキルP閾値表示
	std::unique_ptr<SakuEngine::GameObject2DArray> skillThreshold_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// バー更新
	void UpdateBar();
	// スキルP閾値表示更新
	void UpdateSkillThreshold();
};
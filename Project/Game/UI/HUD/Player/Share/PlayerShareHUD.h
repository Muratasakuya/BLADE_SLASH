#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Material/SpriteVertexColorAnimation.h>

// front
class Player;

//============================================================================
//	PlayerShareHUD class
//	HUDの共有部分
//============================================================================
class PlayerShareHUD {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerShareHUD() = default;
	~PlayerShareHUD() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetPlayer(const Player* player) { player_ = player; }

	// スキル値色が有効か
	bool IsActiveSkillColor() const { return isActiveSkillColor_; }
	// 現在のスキル値色取得
	const SakuEngine::Vector3& GetCurrentSkillColor() const;
	// 現在のスキル閾値表示頂点色取得(α値は返さない)
	const SakuEngine::Color& GetCurrentSkillVertexColor(SakuEngine::SpriteVertexPos pos) const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const Player* player_;

	// スキル値の頂点カラーアニメーション
	std::unique_ptr<SakuEngine::SpriteVertexColorAnimation> skillVertexColorAnim_;
	bool isActiveSkillColor_ = false;       // 現在スキルが有効か
	SakuEngine::Vector3 enableSkillColor_;  // 有効時の色
	SakuEngine::Vector3 disableSkillColor_; // 無効時の色

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// スキル表示色更新
	void UpdateSkillColor();
};
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/SimpleAnimation.h>

// front
class BossEnemy;

//============================================================================
//	BossEnemyStartAnimation class
//	ボスの登場アニメーション
//============================================================================
class BossEnemyStartAnimation {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyStartAnimation() = default;
	~BossEnemyStartAnimation() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// 開始呼び出し
	void Start();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(BossEnemy* bossEnemy) { bossEnemy_ = bossEnemy; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ボス
	BossEnemy* bossEnemy_;

	// 処理が開始されたか
	bool isStarted_;

	// 開始呼び出しからの待機時間
	SakuEngine::StateTimer delayTimer_;
	bool isWaited_;

	// 座標アニメーション
	SakuEngine::SimpleAnimation<SakuEngine::Vector3> posAnimation_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
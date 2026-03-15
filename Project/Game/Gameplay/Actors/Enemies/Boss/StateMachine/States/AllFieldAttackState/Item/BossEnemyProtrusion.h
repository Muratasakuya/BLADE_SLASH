#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/Collider.h>
#include <Engine/Editor/Effect/User/EffectGroup.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	BossEnemyProtrusion class
//============================================================================
class BossEnemyProtrusion {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyProtrusion() = default;
	~BossEnemyProtrusion() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディタ
	void ImGui();

	// 発生
	void Emit(const SakuEngine::Vector3& pos, const SakuEngine::Vector3& direction, float lifeTime);

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		None,   // 何もしていない状態
		Emit,   // 発生処理中
		Wait,   // ライフタイム終了待ち
		Remove, // 削除
	};

	// 発生情報
	struct EmitInfo {

		SakuEngine::Vector3 pos;       // 発生位置
		SakuEngine::Vector3 direction; // 発生方向
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_ = State::None;

	// 連ねる個数
	uint32_t maxProtrusionCount_;
	// すでに発生した個数
	uint32_t emittedProtrusionCount_ = 0;
	// 発生情報
	EmitInfo emitInfo_;

	// 全て発生するのにかかる時間
	SakuEngine::StateTimer emitTimer_;
	// 突起が消えるまでの時間
	SakuEngine::StateTimer lifeTimer_;

	// 突起オブジェクトリスト(エフェクトで出す)
	std::vector<std::unique_ptr<SakuEngine::EffectGroup>> protrusions_;
	// デモ用の突起エフェクト
	std::unique_ptr<SakuEngine::EffectGroup> demoProtrusion_;

	// 発生位置のオフセット距離
	float emitPosOffset_ = 0.0f;
	// 発生位置のYランダム範囲
	float emitPosYRandomRange_ = 0.0f;

	// 衝突判定
	std::unique_ptr<SakuEngine::Collider> collider_;
	SakuEngine::Transform3D collisionTransform_;
	SakuEngine::CollisionBody* collisionBody_;
	// 絶対に当たらない座標
	const SakuEngine::Vector3 collisionSafePos_ = SakuEngine::Vector3(0.0f, -128.0f, 0.0f);

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// エフェクトの作成
	void CreateEffect();
};
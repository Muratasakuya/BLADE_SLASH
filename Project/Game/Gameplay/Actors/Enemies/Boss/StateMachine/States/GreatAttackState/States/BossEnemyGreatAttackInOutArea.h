#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Effect/User/EffectGroup.h>
#include <Engine/Object/Base/KeyframeObject3D.h>
#include <Game/Gameplay/Actors/Enemies/Boss/StateMachine/States/GreatAttackState/Interface/BossEnemyGreatAttackIState.h>

//============================================================================
//	BossEnemyGreatAttackInOutArea class
//============================================================================
class BossEnemyGreatAttackInOutArea :
	public BossEnemyGreatAttackIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackInOutArea();
	~BossEnemyGreatAttackInOutArea() = default;

	// 状態遷移時
	void Enter() override;

	// 更新処理
	void Update() override;
	void UpdateAlways() override;

	// 状態終了時
	void Exit() override;

	// imgui
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		Out, // 外側
		In   // 内側
	};

	// 雷範囲攻撃の衝突情報
	struct RangeAttackCollision {

		std::unique_ptr<SakuEngine::Collider> collider; // 衝突判定

		// 有効フラグ
		bool isActive;
		// トランスフォーム
		SakuEngine::Transform3D transform;
		
		// 発生位置
		SakuEngine::Vector3 activePos;
		// 絶対に当たらない座標
		const SakuEngine::Vector3 collisionSafePos_ = SakuEngine::Vector3(0.0f, -128.0f, 0.0f);

		// 初期化
		void Init();
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	/*---------- In State ----------*/

	// 雷攻撃の発生
	float outAreaRadius_;     // 外側の範囲半径
	uint32_t lightningCount_; // 雷攻撃の個数(半径の分割数)
	static const uint32_t maxLightningCount_ = 32u;

	// 敵の表示を消す
	SakuEngine::StateTimer hideEnemyTimer_;

	// 雷攻撃の時間
	SakuEngine::StateTimer lightningAttackTimer_;

	// 雷攻撃(警告も)
	std::array<std::unique_ptr<SakuEngine::EffectGroup>, maxLightningCount_> lightningAttackEffects_;

	// 雷範囲攻撃の衝突情報
	std::array<RangeAttackCollision, maxLightningCount_> rangeAttackCollisions_;
	// 判定の半径
	float rangeAttackCollisionRadius_;
	// アクティブになるまでの時間
	SakuEngine::StateTimer rangeAttackActiveDelayTimer_;
	// 非アクティブになるまでの時間
	SakuEngine::StateTimer rangeAttackInactiveTimer_;

	/*--------- Out State ----------*/

	// 移動攻撃のキーフレーム
	std::unique_ptr<SakuEngine::KeyframeObject3D> attackKeyframeObject_;
	SakuEngine::StateTimer endWaitTimer_;

	// 攻撃アニメーションに入るまでのキーフレーム
	uint32_t attackKeyframeIndex_;
	uint32_t greatKeyframeIndex_;
	bool isPlayedAttackKeyframe_;
	bool isPlayedGrearAttackAnim_;

	// 大技攻撃への遷移時間
	float grearAttackNextAnimTime_;

	// 最後の攻撃の目標トランスフォーム
	std::unique_ptr<SakuEngine::Transform3D> grearAttackTargetTransform_;

	// 範囲斬撃エフェクト
	std::unique_ptr<SakuEngine::EffectGroup> rangeSlashEffect_;
	bool emitedRangeSlashEffect_; // 発生済みかどうか
	uint32_t rangeSlashKeyIndex_; // 発生させるキーのインデックス
	SakuEngine::Vector3 rangeSlashEffectOffset_; // エフェクトのオフセット
	// 最後の攻撃エフェクト
	std::unique_ptr<SakuEngine::EffectGroup> lastAttackEffect_;
	SakuEngine::Vector3 lastAttackEffectOffset_; // エフェクトのオフセット
	bool emitedLastAttackEffect_; // 発生済みかどうか

	//--------- functions ----------------------------------------------------

	// 状態毎の更新
	void UpdateOut();
	void UpdateIn();

	// 雷攻撃発生
	void EmitLightningAttack();
};
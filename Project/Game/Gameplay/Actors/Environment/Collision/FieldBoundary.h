#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Collision/CollisionCapsule.h>
#include <Engine/Collision/CollisionResolution.h>
#include <Game/Gameplay/Actors/Environment/Collision/FieldWallCollision.h>

//============================================================================
//	FieldBoundary class
//	設定されたオブジェクトをAABBで囲み、範囲外に出た場合に押し戻す処理を行う
//============================================================================
class FieldBoundary :
	public SakuEngine::IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FieldBoundary() :IGameEditor("FieldWallCollisionCollection") {}
	~FieldBoundary() = default;

	// 初期化
	void Init();

	// ゲームプレイ矩形エリア範囲制限
	void ControlQuadPlayArea();
	// 壁衝突体との判定、押し戻し
	void ControlPushBack();

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 押し戻し対象の設定
	void SetPushBackTarget(Player* player, BossEnemy* bossEnemy);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Player* player_;
	BossEnemy* bossEnemy_;

	// AABBの押し戻し領域
	std::vector<std::unique_ptr<FieldWallCollision>> collisions_;
	// 座標移動制限
	float moveClampLength_;

	// カプセル押し戻し設定
	bool useManualCapsuleSolver_ = true;
	bool debugDrawCollision_ = true;

	// カプセル設定
	SakuEngine::CollisionExt::CapsuleSettings playerCapsule_{};
	SakuEngine::CollisionExt::CapsuleSettings bossCapsule_{};
	SakuEngine::CollisionResolve::SolveSettings solveSettings_{};

	// 前フレームの座標
	SakuEngine::Vector3 prevPlayerPos_ = SakuEngine::Vector3::AnyInit(0.0f);
	SakuEngine::Vector3 prevBossPos_ = SakuEngine::Vector3::AnyInit(0.0f);
	bool hasPrevPlayerPos_ = false;
	bool hasPrevBossPos_ = false;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// 全ての衝突体更新
	void UpdateAllCollisionBody();
	// 押し戻し解決
	void SolveTargetsByCapsule();
};
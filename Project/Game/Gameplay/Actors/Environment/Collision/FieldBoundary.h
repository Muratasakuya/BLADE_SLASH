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

	//--------- structure ----------------------------------------------------

	// 球体押し戻し設定
	struct SphereProxySettings {

		SakuEngine::Vector3 centerOffset = SakuEngine::Vector3::AnyInit(0.0f);
		float radius = 0.5f;    // 半径
		bool useXZOnly = true; 	// trueならXZのみで判定

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);
	};
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
	// アクター同士の押し戻し
	void ControlActorPushBack();

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

	//========================================================================
	//	カプセル設定
	//========================================================================

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

	//========================================================================
	//	アクター同士押し戻し設定
	//========================================================================

	// アクター同士押し戻し設定
	bool enableActorPushBack_ = true;
	bool debugDrawActorCollision_ = true;

	SphereProxySettings playerSphere_{};
	SphereProxySettings bossSphere_{};

	// 前フレームの座標
	SakuEngine::Vector3 prevPlayerPosActor_ = SakuEngine::Vector3::AnyInit(0.0f);
	SakuEngine::Vector3 prevBossPosActor_ = SakuEngine::Vector3::AnyInit(0.0f);
	bool hasPrevPlayerPosActor_ = false;
	bool hasPrevBossPosActor_ = false;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// 全ての衝突体更新
	void UpdateAllCollisionBody();
	// 押し戻し解決
	void SolveTargetsByCapsule();
	// アクター同士押し戻し解決
	void SolveTargetsBySpherePair();
};
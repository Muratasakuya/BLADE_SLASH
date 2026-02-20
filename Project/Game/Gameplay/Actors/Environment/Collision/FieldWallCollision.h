#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/Collider.h>

// front(押し戻し対象)
class Player;
class BossEnemy;

//============================================================================
//	FieldWallCollision class
//	任意の場所に設置できる壁衝突判定、押し戻し処理を行う
//============================================================================
class FieldWallCollision :
	public SakuEngine::Collider {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FieldWallCollision() = default;
	~FieldWallCollision() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	/*-------- collision ----------*/

	// 衝突コールバック関数
	void OnCollisionStay(const SakuEngine::CollisionBody* collisionBody) override;

	// editor
	void ImGui(uint32_t index);

	// json
	void ToJson(Json& data);
	void FromJson(const Json& data);

	//--------- accessor -----------------------------------------------------

	// 押し戻し対象の設定
	void SetPushBackTarget(Player* player, BossEnemy* bossEnemy);

	// 手動解決を使う場合は無効化して二重押し戻しを防ぐ
	void SetEnableCallbackPushBack(bool enable) { enableCallbackPushBack_ = enable; }
	bool IsEnableCallbackPushBack() const { return enableCallbackPushBack_; }

	// FieldBoundary側が壁AABBを取得できるようにする
	SakuEngine::CollisionShape::AABB GetWallAABB() const { return GetWorldAABB(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 押し戻し対象
	Player* player_;
	BossEnemy* bossEnemy_;

	// コールバックの多重発火防止フラグ
	bool enableCallbackPushBack_ = true;

	//--------- functions ----------------------------------------------------

	SakuEngine::CollisionShape::AABB GetWorldAABB() const;
	SakuEngine::CollisionShape::AABB MakeAABBProxy(const SakuEngine::CollisionBody* other);
	SakuEngine::Vector3 ComputePushVector(const SakuEngine::CollisionShape::AABB& wall, const SakuEngine::CollisionShape::AABB& actor);
};
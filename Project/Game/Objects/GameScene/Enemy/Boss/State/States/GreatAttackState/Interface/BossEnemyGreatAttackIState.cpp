#include "BossEnemyGreatAttackIState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/SkyboxRenderSystem.h>

//============================================================================
//	BossEnemyGreatAttackIState classMethods
//============================================================================

BossEnemyGreatAttackIState::BossEnemyGreatAttackIState() {

	// skybox取得
	ObjectManager* objectManager = ObjectManager::GetInstance();
	skybox_ = objectManager->GetData<Skybox>(objectManager->GetSystem<SkyboxRenderSystem>()->GetObjectID());
}

void BossEnemyGreatAttackIState::Init(BossEnemy* bossEnemy,
	Player* player, FollowCamera* followCamera, GameLight* gameLight) {

	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;

	player_ = nullptr;
	player_ = player;

	followCamera_ = nullptr;
	followCamera_ = followCamera;

	gameLight_ = nullptr;
	gameLight_ = gameLight;
}
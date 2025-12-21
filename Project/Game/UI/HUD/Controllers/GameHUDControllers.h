#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Event/GameEventBus/GameEventBus.h>
#include <Game/Event/GameHUDEvents/GameHUDEvents.h>
#include <Game/UI/HUD/Interface/IGameHUDController.h>

// player
#include <Game/UI/HUD/Player/PlayerHUD.h>
#include <Game/UI/HUD/Player/PlayerStunHUD.h>
#include <Game/UI/HUD/Player/TargetNavigation/TargetNavigation.h>
// boss
#include <Game/UI/HUD/Enemy/Boss/BossEnemyHUD.h>

// c++
#include <optional>
#include <memory>
// front
class Player;
class BossEnemy;
class FollowCamera;

//============================================================================
//	GameHUDControllers class
//	クラスの説明がここに入る
//============================================================================
class GameHUDControllers {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameHUDControllers() = default;
	~GameHUDControllers() = default;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------



	//--------- functions ----------------------------------------------------

};
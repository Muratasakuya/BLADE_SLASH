#pragma once

//============================================================================
//	include
//============================================================================

// scene
#include <Game/Gameplay/Camera/CameraManager.h>
#include <Game/Gameplay/Lighting/GameLight.h>
#include <Game/Gameplay/Actors/Environment/Collision/FieldBoundary.h>

// object
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Result/GameResultDisplay.h>

// sprite
#include <Game/Effects/SpriteEffect/FadeSprite.h>

//============================================================================
//	GameContext
//============================================================================

// シーン内で動かすもの
struct GameContext {

	// scene
	CameraManager* camera = nullptr;
	GameLight* light = nullptr;
	FieldBoundary* fieldBoundary = nullptr;

	// object
	Player* player = nullptr;
	BossEnemy* boss = nullptr;
	GameResultDisplay* result = nullptr;

	// sprite
	FadeSprite* fadeSprite = nullptr;
};
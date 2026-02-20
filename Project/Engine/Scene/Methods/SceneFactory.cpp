#include "SceneFactory.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Game/Scenes/Debug/DebugScene.h>
#include <Game/Scenes/Title/TitleScene.h>
#include <Game/Scenes/Game/GameScene.h>

//============================================================================
//	SceneFactory classMethods
//============================================================================

std::unique_ptr<IScene> SceneFactory::Create(Scene scene) {

	switch (scene) {
	case Scene::Debug:

		return std::make_unique<DebugScene>();
	case Scene::Title:

		return std::make_unique<TitleScene>();
	case Scene::Game:

		return std::make_unique<GameScene>();
	}
	return nullptr;
}

#include "SceneFactory.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Game/Scenes/Title/TitleScene.h>
#include <Game/Scenes/Game/GameScene.h>
#include <Game/Scenes/Effect/EffectScene.h>

//============================================================================
//	SceneFactory classMethods
//============================================================================

std::unique_ptr<IScene> SceneFactory::Create(Scene scene) {

	switch (scene) {
	case Scene::Effect:

		return std::make_unique<EffectScene>();
	case Scene::Title:

		return std::make_unique<TitleScene>();
	case Scene::Game:

		return std::make_unique<GameScene>();
	}
	return nullptr;
}

#include "SceneFactory.h"

//============================================================================
//	include
//============================================================================
#include <Game/Scene/EffectScene.h>
#include <Game/Scene/TitleScene.h>
#include <Game/Scene/GameScene.h>

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
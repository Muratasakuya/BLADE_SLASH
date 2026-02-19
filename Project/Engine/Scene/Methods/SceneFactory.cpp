#include "SceneFactory.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Game/Scenes/Debug/DebugScene.h>

//============================================================================
//	SceneFactory classMethods
//============================================================================

std::unique_ptr<IScene> SceneFactory::Create(Scene scene) {

	switch (scene) {
	case Scene::Debug:

		return std::make_unique<DebugScene>();
	}
	return nullptr;
}

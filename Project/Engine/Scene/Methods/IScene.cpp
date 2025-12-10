#include "IScene.h"

//============================================================================
//	IScene classMethods
//============================================================================

void IScene::SetPtr(SceneView* sceneView, SceneManager* sceneManager) {

	sceneView_ = nullptr;
	sceneView_ = sceneView;

	sceneManager_ = nullptr;
	sceneManager_ = sceneManager;
}
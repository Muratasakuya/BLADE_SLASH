#include "DebugScene.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/SceneView.h>
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>

//============================================================================
//	DebugScene classMethods
//============================================================================

void DebugScene::Init() {

	debugText_ = std::make_unique<SakuEngine::GameTextObject>();
	debugText_->Init("DS-DIGIB_msdf", "Assets/Json/Atlas/DS-DIGIB_msdf.json", "debugText", "Text");

	//========================================================================
	//	scene
	//========================================================================

	// カメラの設定
	camera3D_ = std::make_unique<SakuEngine::BaseCamera>();
	sceneView_->SetGameCamera(camera3D_.get());

	// ライトの設定
	light_ = std::make_unique<SakuEngine::BasePunctualLight>();
	light_->Init();
	sceneView_->SetLight(light_.get());
}

void DebugScene::Update() {

	camera3D_->UpdateView();
}
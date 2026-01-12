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
	debugText_->Init("FiraMono_msdf", "Assets/Json/Atlas/FiraMono_msdf.json", "debugText", "Text");

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

	SakuEngine::LineRenderer::GetInstance()->Get3D()->DrawGrid(32, 32.0f, SakuEngine::Color::White());
}
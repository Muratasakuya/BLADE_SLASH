#include "ResultGameState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>

//============================================================================
//	ResultGameState classMethods
//============================================================================

void ResultGameState::Enter() {

	//========================================================================
	//	postProcess
	//========================================================================

	SakuEngine::PostProcessSystem* postProcessSystem = SakuEngine::PostProcessSystem::GetInstance();
	postProcessSystem->ClearProcess();
	postProcessSystem->AddProcess(PostProcessType::Grayscale);

	//========================================================================
	//	object
	//========================================================================

	// リザルト画面表示開始
	context_->result->StartDisplay();

	// 入った瞬間から確実に非表示
	context_->inputController->ResultUpdateMouse(false);
}

void ResultGameState::Init([[maybe_unused]] SakuEngine::SceneView* sceneView) {
}

void ResultGameState::Update() {

	const GameSceneState currentState = GameSceneState::Result;

	//========================================================================
	//	object
	//========================================================================

	// リザルト画面更新
	context_->result->Update();

	// 入力デバイスコントローラー更新
	context_->inputController->ResultUpdateMouse(context_->result->IsDisplayFinished());

	//========================================================================
	//	scene
	//========================================================================

	// リザルト用のカメラを更新
	context_->camera->Update(currentState);

	//========================================================================
	//	sceneEvent
	//========================================================================

	// 入力に応じて遷移先を決定する
	if (context_->result->GetResultSelect() != ResultSelect::None) {

		requestNext_ = true;
	}
}

void ResultGameState::NonActiveUpdate() {
}
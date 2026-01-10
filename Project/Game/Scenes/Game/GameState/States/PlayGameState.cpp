#include "PlayGameState.h"

//============================================================================
//	PlayGameState classMethods
//============================================================================

void PlayGameState::Init([[maybe_unused]] SakuEngine::SceneView* sceneView) {

	nextState_ = GameSceneState::EndGame;
}

void PlayGameState::Update() {

	// 入力デバイスコントローラー更新
	context_->inputController->Update();

	const GameSceneState currentState = GameSceneState::PlayGame;

	//========================================================================
	//	object
	//========================================================================

	context_->boss->Update(currentState);
	context_->player->Update();

	// 時間経過を計測
	context_->result->Measurement();

	//========================================================================
	//	hud
	//========================================================================

	context_->hudSynchronizer->Update();

	//========================================================================
	//	sprite
	//========================================================================

	// 遷移後処理
	context_->fadeSprite->Update();

	//========================================================================
	//	scene
	//========================================================================

	// 移動範囲を制限する
	context_->fieldBoundary->ControlQuadPlayArea();
	// アクター同士の押し戻しを行う
	context_->fieldBoundary->ControlActorPushBack();

	// カメラを更新する
	context_->camera->Update(currentState);

	//========================================================================
	//	sceneEvent
	//========================================================================

	// プレイヤーか敵が死んだらクリアシーンに遷移させる
	if (context_->player->IsDead() ||
		context_->boss->IsDead()) {

		// フェード開始
		requestNext_ = true;
	}
}

void PlayGameState::NonActiveUpdate() {
}

void PlayGameState::Enter() {
}

void PlayGameState::ImGui() {

	context_->fieldBoundary->ImGui();
}
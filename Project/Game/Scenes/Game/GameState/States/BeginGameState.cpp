#include "BeginGameState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Engine/Editor/Camera/CameraEditor.h>

//============================================================================
//	BeginGameState classMethods
//============================================================================

void BeginGameState::Init([[maybe_unused]] SakuEngine::SceneView* sceneView) {

	// 次の状態を設定
	nextState_ = GameSceneState::PlayGame;

	// ゲーム開始時のカメラアニメーションデータを読み込み
	SakuEngine::CameraEditor::GetInstance()->LoadJson("Scene/startGameCamera.json");

	// json適用
	ApplyJson();
}

void BeginGameState::Update() {

	// 入力デバイスコントローラー更新
	context_->inputController->Update();

	const GameSceneState currentState = GameSceneState::BeginGame;

	//========================================================================
	//	object
	//========================================================================

	context_->boss->Update(currentState);

	//========================================================================
	//	hud
	//========================================================================

	context_->hudSynchronizer->Update();

	//========================================================================
	//	scene
	//========================================================================

	context_->camera->Update(currentState);

	//========================================================================
	//	sceneEvent
	//========================================================================

	// ボスの登場演出が終了したらゲーム開始
	if (SakuEngine::CameraEditor::GetInstance()->IsAnimFinished()) {

		context_->fadeSprite->Start();
		// 遷移処理
		context_->fadeSprite->Update();
	}
	// 次の状態に遷移
	if (context_->fadeSprite->IsFinished()) {

		requestNext_ = true;
	}
}

void BeginGameState::NonActiveUpdate() {
}

void BeginGameState::Enter() {

	// アイドル状態に戻す
	context_->player->Update();
	context_->player->ResetState();

	// プレイヤーの座標をゲーム開始座標にする
	context_->player->SetTranslation(startPlayerPos_);

	// カメラのアニメーション開始
	SakuEngine::CameraEditor::GetInstance()->StartAnim("startGameCamera", false);
}

void BeginGameState::Exit() {

	// エディターによる更新を止めさせる
	context_->camera->GetFollowCamera()->SetIsUpdateEditor(false);
}

void BeginGameState::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::DragFloat3("startPlayerPos", &startPlayerPos_.x, 0.1f);
	SakuEngine::LineRenderer::GetInstance()->Get3D()->DrawSphere(8, 8.0f, startPlayerPos_,
		SakuEngine::Color::Red());
}

void BeginGameState::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Scene/State/beginGameState.json", data)) {
		return;
	}

	startPlayerPos_ = SakuEngine::Vector3::FromJson(data["startPlayerPos_"]);
}

void BeginGameState::SaveJson() {

	Json data;

	data["startPlayerPos_"] = startPlayerPos_.ToJson();

	SakuEngine::JsonAdapter::Save("Scene/State/beginGameState.json", data);
}
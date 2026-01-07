#include "StartGameState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Scene/SceneView.h>

//============================================================================
//	StartGameState classMethods
//============================================================================

void StartGameState::Init(SakuEngine::SceneView* sceneView) {

	// 次の状態を設定
	nextState_ = GameSceneState::BeginGame;

	//========================================================================
	//	postProcess
	//========================================================================

	SakuEngine::PostProcessSystem* postProcessSystem = SakuEngine::PostProcessSystem::GetInstance();
	postProcessSystem->AddProcess(PostProcessType::RadialBlur);
	postProcessSystem->AddProcess(PostProcessType::PlayerAfterImage);
	postProcessSystem->AddProcess(PostProcessType::DepthBasedOutline);
	postProcessSystem->AddProcess(PostProcessType::Grayscale);
	postProcessSystem->AddProcess(PostProcessType::Bloom);

	//========================================================================
	//	sceneObject
	//========================================================================

	// カメラ
	context_->camera->Init(sceneView);

	// ライトの初期設定
	context_->light->Init();
	context_->light->Start();
	sceneView->SetLight(context_->light);

	// 衝突
	context_->fieldBoundary->Init();

	//========================================================================
	//	frontObjects
	//========================================================================

	// ボス
	context_->boss->Init("bossEnemy", "bossEnemy", "Enemy", "bossEnemy_idle");

	// プレイヤー
	context_->player->Init("player", "player", "Player", "player_idle");

	// 必要なデータをセット
	context_->boss->SetPlayer(context_->player);
	context_->boss->SetFollowCamera(context_->camera->GetFollowCamera());
	context_->player->SetBossEnemy(context_->boss);
	context_->player->SetFollowCamera(context_->camera->GetFollowCamera());

	// 衝突応答にプレイヤー、ボスをセット
	context_->fieldBoundary->SetPushBackTarget(context_->player, context_->boss);

	// リザルト画面
	context_->result->Init();

	// 追従先を設定する
	context_->camera->SetTarget(context_->player, context_->boss);

	//========================================================================
	//	hud
	//========================================================================

	// HUD同期
	context_->hudSynchronizer->Init(context_->player, context_->boss,
		context_->camera->GetFollowCamera());

	//========================================================================
	//	sprites
	//========================================================================

	context_->fadeSprite->Init("white", "fadeSprite", "Scene");

	//========================================================================
	//	sceneEvent
	//========================================================================

	nextStateEvent_ = std::make_unique<SakuEngine::Collider>();
	// 衝突タイプ設定
	SakuEngine::CollisionBody* body = nextStateEvent_->AddCollider(SakuEngine::CollisionShape::AABB().Default(), true);
	// タイプ設定
	body->SetType(ColliderType::Type_Event);
	body->SetTargetType(ColliderType::Type_Player);

	// json適用
	ApplyJson();
}

void StartGameState::Update() {

	const GameSceneState currentState = GameSceneState::Start;

	//========================================================================
	//	object
	//========================================================================

	context_->player->Update();
	context_->boss->Update(currentState);

	//========================================================================
	//	hud
	//========================================================================

	context_->hudSynchronizer->Update();

	//========================================================================
	//	scene
	//========================================================================

	// 移動範囲を制限する
	context_->fieldBoundary->ControlPushBack();

	// カメラを更新する
	context_->camera->Update(currentState);

	//========================================================================
	//	sceneEvent
	//========================================================================

	SakuEngine::Transform3D transform{};
	transform.scale = SakuEngine::Vector3::AnyInit(1.0f);
	nextStateEvent_->UpdateAllBodies(transform);

	// イベント範囲内に入ったら次の状態に遷移させる
	if (nextStateEvent_->IsHitTrigger()) {

		requestNext_ = true;
	}
}

void StartGameState::NonActiveUpdate() {
}

void StartGameState::Exit() {
}

void StartGameState::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	nextStateEvent_->ImGui(192.0f);
}

void StartGameState::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Scene/State/startGameState.json", data)) {
		return;
	}

	nextStateEvent_->ApplyBodyOffset(data["NextStateEvent"]);
}

void StartGameState::SaveJson() {

	Json data;

	nextStateEvent_->SaveBodyOffset(data["NextStateEvent"]);

	SakuEngine::JsonAdapter::Save("Scene/State/startGameState.json", data);
}
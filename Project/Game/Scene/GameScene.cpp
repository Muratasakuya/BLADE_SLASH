#include "GameScene.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Scene/Manager/SceneManager.h>
#include <Engine/Editor/Camera/CameraEditor.h>

// scene
#include <Game/Scene/GameState/States/StartGameState.h>
#include <Game/Scene/GameState/States/BeginGameState.h>
#include <Game/Scene/GameState/States/PlayGameState.h>
#include <Game/Scene/GameState/States/EndGameState.h>
#include <Game/Scene/GameState/States/ResultGameState.h>
#include <Game/Scene/GameState/States/PauseState.h>

//============================================================================
//	GameScene classMethods
//============================================================================

GameScene::~GameScene() {

	// シーンを削除するタイミングで停止させる
	fieldEffect_->Stop();
	// カメラキーデータをクリア
	SakuEngine::CameraEditor::GetInstance()->ResetAllKeyData();
}

void GameScene::InitStates() {

	// scene
	context_.camera = cameraManager_.get();
	context_.light = gameLight_.get();
	context_.fieldBoundary = fieldBoundary_.get();
	// object
	context_.player = player_.get();
	context_.subPlayer = subPlayer_.get();
	context_.boss = bossEnemy_.get();
	context_.result = result_.get();
	// sprite
	context_.fadeSprite = fadeSprite_.get();

	// シーン状態クラスの初期化
	states_[static_cast<uint32_t>(GameSceneState::Start)] = std::make_unique<StartGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::Start)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::BeginGame)] = std::make_unique<BeginGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::BeginGame)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::PlayGame)] = std::make_unique<PlayGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::PlayGame)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::EndGame)] = std::make_unique<EndGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::EndGame)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::Result)] = std::make_unique<ResultGameState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::Result)]->Init(sceneView_);

	states_[static_cast<uint32_t>(GameSceneState::Pause)] = std::make_unique<PauseState>(&context_);
	states_[static_cast<uint32_t>(GameSceneState::Pause)]->Init(sceneView_);

	// 最初の状態を設定
	currentState_ = GameSceneState::Start;
}

void GameScene::Init() {

	//========================================================================
	//	sceneObjects
	//========================================================================

	cameraManager_ = std::make_unique<CameraManager>();
	gameLight_ = std::make_unique<GameLight>();
	fieldBoundary_ = std::make_unique<FieldBoundary>();

	//========================================================================
	//	frontObjects
	//========================================================================

	player_ = std::make_unique<Player>();
	subPlayer_ = std::make_unique<SubPlayer>();
	bossEnemy_ = std::make_unique<BossEnemy>();
	result_ = std::make_unique<GameResultDisplay>();

	fadeSprite_ = std::make_unique<FadeSprite>();

	// フィールドエフェクト
	fieldEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	fieldEffect_->Init("fieldEffect", "SceneEffect");
	fieldEffect_->LoadJson("GameEffectGroup/GameScene/sceneFieldEffect.json");
	// 最初から発生
	fieldEffect_->Emit(SakuEngine::Vector3::AnyInit(0.0f));

	//========================================================================
	//	state
	//========================================================================

	// シーン状態の初期化
	InitStates();

	// 遷移の設定
	fadeTransition_ = std::make_unique<FadeTransition>();
	fadeTransition_->Init();
}

void GameScene::Update() {

	// 状態に応じて更新
	uint32_t stateIndex = static_cast<uint32_t>(currentState_);
	// 現在の状態を更新
	states_[stateIndex]->Update();

	// 状態が終了次第次の状態へ遷移
	if (states_[stateIndex]->IsRequestNext()) {
		if (states_[stateIndex]->GetNextState() == GameSceneState::Result) {

			// プレイヤーと敵を消す
			player_.reset();
			subPlayer_.reset();
			bossEnemy_.reset();
		}
		if (currentState_ == GameSceneState::Result) {

			// 入力に応じて遷移先を決定する
			if (fadeTransition_) {
				if (states_[stateIndex]->GetResultSelect() == ResultSelect::Retry) {

					sceneManager_->SetNextScene(SakuEngine::Scene::Game, std::move(fadeTransition_));
				} else if (states_[stateIndex]->GetResultSelect() == ResultSelect::Title) {

					sceneManager_->SetNextScene(SakuEngine::Scene::Title, std::move(fadeTransition_));
				}
			}
		} else {

			// 次の状態へ遷移
			RequestNextState(states_[stateIndex]->GetNextState());
		}
	}

	// フィールドエフェクト更新
	fieldEffect_->Update();
}

void GameScene::EndFrame() {

	// カメラアニメーションの読み込み
	context_.camera->GetFollowCamera()->LoadAnim();
}

void GameScene::RequestNextState(GameSceneState next) {

	// 現在の状態を終了させる
	uint32_t stateIndex = static_cast<uint32_t>(currentState_);
	states_[stateIndex]->Exit();

	// 遷移
	currentState_ = next;
	uint32_t nextIndex = static_cast<uint32_t>(next);

	// 次の状態を設定
	states_[nextIndex]->Enter();
	states_[stateIndex]->ClearRequestNext();
}

void GameScene::ImGui() {

	ImGui::SeparatorText(SakuEngine::EnumAdapter<GameSceneState>::ToString(currentState_));

	uint32_t stateIndex = static_cast<uint32_t>(currentState_);
	states_[stateIndex]->ImGui();

	ImGui::SeparatorText("Edit State");

	GameSceneState nextState = currentState_;

	// コンボでnextStateを変更
	if (SakuEngine::EnumAdapter<GameSceneState>::Combo("SceneState", &nextState)) {

		// 実際に状態が変わったときだけ遷移する
		if (nextState != currentState_) {

			RequestNextState(nextState);
		}
	}
}
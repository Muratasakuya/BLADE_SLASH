#include "FollowCameraUpdatePass.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/CameraEditor.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>

// passes
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Composer/FollowCameraOrbitTranslationComposer.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Rotation/FollowCameraLookRotationIntegrator.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Rotation/FollowCameraPitchClamper.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Rotation/FollowCameraRollStabilizer.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Input/FollowCameraLookInputSmoother.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Target/FollowCameraTargetResolver.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Target/FollowCameraInterTargetSmoother.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Target/FollowCameraActionAutoLookTarget.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Offset/FollowCameraZoomOffsetResolver.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Offset/FollowCameraOffsetSmoother.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Fov/FollowCameraCalFov.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Fov/FollowCameraReturnFov.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Editor/FollowCameraReturnToFollowSmoother.h>

//============================================================================
//	FollowCameraUpdatePass classMethods
//============================================================================

void FollowCameraUpdatePass::Init() {

	auto& registry = PassRegistry::GetInstance();

	// 更新パスを追加
	registry.Register<FollowCameraLookInputSmoother>();
	registry.Register<FollowCameraTargetResolver>();
	registry.Register<FollowCameraInterTargetSmoother>();
	registry.Register<FollowCameraLookRotationIntegrator>();
	registry.Register<FollowCameraPitchClamper>();
	registry.Register<FollowCameraRollStabilizer>();
	registry.Register<FollowCameraZoomOffsetResolver>();
	registry.Register<FollowCameraOffsetSmoother>();
	registry.Register<FollowCameraOrbitTranslationComposer>();
	registry.Register<FollowCameraCalFov>();
	registry.Register<FollowCameraReturnFov>();
	registry.Register<FollowCameraActionAutoLookTarget>();
	registry.Register<FollowCameraReturnToFollowSmoother>();

	// 実行順
	static constexpr std::array<FollowCameraUpdatePassID, 13> kExecuteOrder = {

		FollowCameraUpdatePassID::LookInputSmoother,        // 入力
		FollowCameraUpdatePassID::TargetResolver,           // 追従ターゲット決定
		FollowCameraUpdatePassID::InterTargetSmoother,      // interTarget 補間

		FollowCameraUpdatePassID::LookRotationIntegrator,   // 入力回転の積分
		FollowCameraUpdatePassID::ActionAutoLookTarget,     // 自動ロックオン等
		FollowCameraUpdatePassID::PitchClamper,             // ピッチ制限

		FollowCameraUpdatePassID::ZoomOffsetResolver,       // 回転に依存するオフセット計算
		FollowCameraUpdatePassID::OffsetSmoother,           // オフセット補間
		FollowCameraUpdatePassID::OrbitTranslationComposer, // 最終座標合成

		FollowCameraUpdatePassID::RollStabilizer,   // ロール安定化
		FollowCameraUpdatePassID::CalFov,           // デフォルト画角計算
		FollowCameraUpdatePassID::ReturnFov,        // 画角戻し
		FollowCameraUpdatePassID::ToFollowSmoother, // エディター戻し
	};

	// 登録されているパスを生成して格納
	updatePasses_.clear();
	updatePasses_.reserve(kExecuteOrder.size());

	for (FollowCameraUpdatePassID id : kExecuteOrder) {

		std::unique_ptr<IFollowCameraUpdatePass> pass = registry.Create(id);
		pass->Init();
		updatePasses_.emplace_back(std::move(pass));
	}

	// サービスを構築
	BuildFrameService();
}

void FollowCameraUpdatePass::BindDependencies(const FollowCameraDependencies& dependencies) {

	// 各パスに依存オブジェクトを設定
	for (const auto& pass : updatePasses_) {

		pass->BindDependencies(dependencies);
	}
}

void FollowCameraUpdatePass::BuildFrameService() {

	// 処理を行うパスの数に変更があった場合にのみ構築にする予定
	frameService_.targetResolver = static_cast<FollowCameraTargetResolver*>(GetPassByID(FollowCameraUpdatePassID::TargetResolver));
	frameService_.rotationIntegrator = static_cast<FollowCameraLookRotationIntegrator*>(GetPassByID(FollowCameraUpdatePassID::LookRotationIntegrator));
	frameService_.zoomOffsetResolver = static_cast<FollowCameraZoomOffsetResolver*>(GetPassByID(FollowCameraUpdatePassID::ZoomOffsetResolver));
	frameService_.offsetSmoother = static_cast<FollowCameraOffsetSmoother*>(GetPassByID(FollowCameraUpdatePassID::OffsetSmoother));
	frameService_.inputSmoother = static_cast<FollowCameraLookInputSmoother*>(GetPassByID(FollowCameraUpdatePassID::LookInputSmoother));
	frameService_.pitchClamper = static_cast<FollowCameraPitchClamper*>(GetPassByID(FollowCameraUpdatePassID::PitchClamper));
	frameService_.toFollowSmoother = static_cast<FollowCameraReturnToFollowSmoother*>(GetPassByID(FollowCameraUpdatePassID::ToFollowSmoother));
	frameService_.calFov = static_cast<FollowCameraCalFov*>(GetPassByID(FollowCameraUpdatePassID::CalFov));
}

IFollowCameraUpdatePass* FollowCameraUpdatePass::GetPassByID(FollowCameraUpdatePassID id) {

	for (const auto& pass : updatePasses_) {
		if (pass->GetID() == id) {
			return pass.get();
		}
	}
	return nullptr;
}

void FollowCameraUpdatePass::Update(FollowCamera& followCamera) {

	// コンテキストを初期化
	context_.cameraTranslation = followCamera.GetTransform().translation;
	context_.cameraRotation = followCamera.GetTransform().rotation;
	context_.cameraFovY = followCamera.GetFovY();

	// デルタタイム取得
	float deltaTime = GameTimer::GetDeltaTime();
	for (auto& pass : updatePasses_) {

		// 開始処理
		pass->Begin(context_);
		// 実行
		pass->Execute(context_, frameService_, deltaTime);
	}

	// エディターによる更新中は反映しない
	if (followCamera.IsUpdateEditor()) {
		return;
	}

	// コンテキストの値をカメラに反映
	followCamera.SetTranslation(context_.cameraTranslation);
	followCamera.SetRotation(context_.cameraRotation);
	followCamera.SetFovY(context_.cameraFovY);
}

void FollowCameraUpdatePass::ImGui() {

	if (ImGui::BeginTabBar("CameraUpdatePass")) {

		//=========================================================================================================================
		//	更新パスコンフィグ
		//=========================================================================================================================

		if (ImGui::BeginTabItem("Config")) {
			

			ImGui::EndTabItem();
		}

		//=========================================================================================================================
		//	更新パスの編集
		//=========================================================================================================================
		if (ImGui::BeginTabItem("Edit Pass")) {

			EnumAdapter<FollowCameraUpdatePassID>::Combo("Edit UpdatePass", &editPassID_);

			ImGui::SeparatorText(EnumAdapter<FollowCameraUpdatePassID>::ToString(editPassID_));

			for (const auto& pass : updatePasses_) {
				if (pass->GetID() == editPassID_) {

					pass->ImGui();
					break;
				}
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
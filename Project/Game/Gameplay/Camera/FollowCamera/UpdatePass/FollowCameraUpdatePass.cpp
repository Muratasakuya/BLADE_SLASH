#include "FollowCameraUpdatePass.h"

//============================================================================
//	include
//============================================================================
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
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Offset/FollowCameraZoomOffsetResolver.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Offset/FollowCameraOffsetSmoother.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Fov/FollowCameraReturnFov.h>

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
	registry.Register<FollowCameraReturnFov>();

	// 登録されているパスを生成して格納
	for (const auto& name : SakuEngine::EnumAdapter<FollowCameraUpdatePassID>::GetEnumArray()) {

		// 名前をIDに変換
		FollowCameraUpdatePassID id = SakuEngine::EnumAdapter<FollowCameraUpdatePassID>::FromString(name).value();
		// 生成して初期化
		std::unique_ptr<IFollowCameraUpdatePass> pass = registry.Create(id);
		pass->Init();
		// 格納
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

	for (auto& pass : updatePasses_) {

		// 開始処理
		pass->Begin(context_);
		// 実行
		pass->Execute(context_, frameService_, SakuEngine::GameTimer::GetDeltaTime());
	}

	// コンテキストの値をカメラに反映
	followCamera.SetTranslation(context_.cameraTranslation);
	followCamera.SetRotation(context_.cameraRotation);
	followCamera.SetFovY(context_.cameraFovY);
}

void FollowCameraUpdatePass::ImGui() {

	SakuEngine::EnumAdapter<FollowCameraUpdatePassID>::Combo("Edit UpdatePass", &editPassID_);

	ImGui::SeparatorText(SakuEngine::EnumAdapter<FollowCameraUpdatePassID>::ToString(editPassID_));

	for (const auto& pass : updatePasses_) {
		if (pass->GetID() == editPassID_) {

			pass->ImGui();
			break;
		}
	}
}
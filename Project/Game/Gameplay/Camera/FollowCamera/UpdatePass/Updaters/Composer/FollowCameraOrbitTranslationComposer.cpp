#include "FollowCameraOrbitTranslationComposer.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Updaters/Offset/FollowCameraOffsetSmoother.h>

//============================================================================
//	FollowCameraOrbitTranslationComposer classMethods
//============================================================================

void FollowCameraOrbitTranslationComposer::Execute(FollowCameraContext& context,
	const FollowCameraFrameService& service, [[maybe_unused]] float deltaTime) {

	// オフセット補間クラスから現在のオフセット距離を取得
	SakuEngine::Vector3 offset = service.offsetSmoother->GetCurrentOffset();
	context.cameraTranslation = context.interTarget + context.cameraRotation * offset;
}
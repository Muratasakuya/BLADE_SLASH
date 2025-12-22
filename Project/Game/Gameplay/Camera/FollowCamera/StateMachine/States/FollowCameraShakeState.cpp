//#include "FollowCameraShakeState.h"
//
////============================================================================
////	include
////============================================================================
//#include <Engine/Utility/Timer/GameTimer.h>
//#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
//#include <Engine/Utility/Random/RandomGenerator.h>
//#include <Engine/Utility/Json/JsonAdapter.h>
//
////============================================================================
////	FollowCameraShakeState classMethods
////============================================================================
//
//FollowCameraShakeState::FollowCameraShakeState() {
//
//	canExit_ = false;
//}
//
//void FollowCameraShakeState::Enter([[maybe_unused]] FollowCamera& followCamera) {
//}
//
//void FollowCameraShakeState::Update(FollowCamera& followCamera) {
//
//	shakeTimer_ += SakuEngine::GameTimer::GetScaledDeltaTime();
//	const SakuEngine::Transform3D& transform = followCamera.GetTransform();
//
//	// シェイクの残り時間を計算
//	if (shakeTimer_ >= shakeTime_) {
//		canExit_ = true;
//		return;
//	}
//	// 時間経過で減衰させる
//	float lerpT = shakeTimer_ / shakeTime_;
//	lerpT = EasedValue(shakeEasingType_, lerpT);
//	float intensity = std::lerp(shakeXZIntensity_, 0.0f, lerpT);
//	float offsetYIntensity = std::lerp(shakeOffsetYIntensity_, 0.0f, lerpT);
//
//	float offsetX = SakuEngine::RandomGenerator::Generate(-1.0f, 1.0f) * intensity;
//	float offsetY = SakuEngine::RandomGenerator::Generate(-1.0f, 1.0f) * (intensity + offsetYIntensity);
//	float offsetZ = SakuEngine::RandomGenerator::Generate(-1.0f, 1.0f) * intensity;
//
//	SakuEngine::Vector3 forward = transform.GetForward();
//	SakuEngine::Vector3 right = transform.GetRight();
//
//	SakuEngine::Vector3 translation = transform.translation + (forward * offsetZ) + (right * offsetX);
//	translation.y += offsetY;
//
//	// 座標を設定
//	followCamera.SetTranslation(translation);
//}
//
//void FollowCameraShakeState::Exit() {
//
//	// リセット
//	shakeTimer_ = 0.0f;
//	canExit_ = false;
//}
//
//void FollowCameraShakeState::ImGui([[maybe_unused]] const FollowCamera& followCamera) {
//
//	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
//	ImGui::Text(std::format("shakeTimer: {}", shakeTimer_).c_str());
//	ImGui::DragFloat("shakeXZIntensity", &shakeXZIntensity_, 0.01f);
//	ImGui::DragFloat("shakeOffsetYIntensity", &shakeOffsetYIntensity_, 0.01f);
//	ImGui::DragFloat("shakeTime", &shakeTime_, 0.01f);
//	Easing::SelectEasingType(shakeEasingType_, "shakeEasingType_");
//}
//
//void FollowCameraShakeState::ApplyJson(const Json& data) {
//
//	shakeXZIntensity_ = SakuEngine::JsonAdapter::GetValue<float>(data, "shakeXZIntensity_");
//	shakeOffsetYIntensity_ = SakuEngine::JsonAdapter::GetValue<float>(data, "shakeOffsetYIntensity_");
//	shakeTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "shakeTime_");
//	shakeEasingType_ = static_cast<EasingType>(SakuEngine::JsonAdapter::GetValue<int>(data, "shakeEasingType_"));
//}
//
//void FollowCameraShakeState::SaveJson(Json& data) {
//
//	data["shakeXZIntensity_"] = shakeXZIntensity_;
//	data["shakeOffsetYIntensity_"] = shakeOffsetYIntensity_;
//	data["shakeTime_"] = shakeTime_;
//	data["shakeEasingType_"] = static_cast<int>(shakeEasingType_);
//}
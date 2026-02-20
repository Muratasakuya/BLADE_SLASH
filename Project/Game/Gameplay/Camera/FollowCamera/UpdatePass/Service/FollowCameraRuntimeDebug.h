#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>

//============================================================================
//	FollowCameraRuntimeDebug structures
//============================================================================

// パス毎のランタイム差分データ
struct FollowCameraRuntimePassDelta {

	FollowCameraUpdatePassID id{};

	SakuEngine::Vector3 camPosBefore{};
	SakuEngine::Vector3 camPosAfter{};
	SakuEngine::Quaternion camRotBefore = SakuEngine::Quaternion::Identity();
	SakuEngine::Quaternion camRotAfter = SakuEngine::Quaternion::Identity();
	float camFovBefore = 0.0f;
	float camFovAfter = 0.0f;
	SakuEngine::Vector3 interTargetBefore{};
	SakuEngine::Vector3 interTargetAfter{};

	SakuEngine::Vector3 dCamPos{};
	float dCamPosLen = 0.0f;
	float dRotDeg = 0.0f;
	float dFov = 0.0f;
	SakuEngine::Vector3 dInterTarget{};
	float dInterTargetLen = 0.0f;

	float score = 0.0f;
};

// ランタイム差分集計データ
struct FollowCameraRuntimeDeltaSummary {

	FollowCameraUpdatePassID maxPosID = FollowCameraUpdatePassID::LookInputSmoother;
	float maxPos = 0.0f;

	FollowCameraUpdatePassID maxRotID = FollowCameraUpdatePassID::LookInputSmoother;
	float maxRotDeg = 0.0f;

	FollowCameraUpdatePassID maxFovID = FollowCameraUpdatePassID::LookInputSmoother;
	float maxFov = 0.0f;

	FollowCameraUpdatePassID maxInterTargetID = FollowCameraUpdatePassID::LookInputSmoother;
	float maxInterTarget = 0.0f;

	FollowCameraUpdatePassID maxScoreID = FollowCameraUpdatePassID::LookInputSmoother;
	float maxScore = 0.0f;
};
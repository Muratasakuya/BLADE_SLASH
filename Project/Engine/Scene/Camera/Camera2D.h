#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>

//============================================================================
//	Camera2D class
//	2D視点カメラ
//============================================================================
namespace SakuEngine {

class Camera2D :
	public SakuEngine::BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Camera2D() = default;
	~Camera2D() = default;

	void Init() override;

	void Update() override {}

	void ImGui() override;
};

}; // SakuEngine

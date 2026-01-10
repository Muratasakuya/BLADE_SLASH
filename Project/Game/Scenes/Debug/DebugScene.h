#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Methods/IScene.h>

// scene
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Scene/Light/PunctualLight.h>

// デバッグに使用するクラス

//============================================================================
//	DebugScene class
//	機能のデバッグ確認を行う用のシーン
//============================================================================
class DebugScene :
	public SakuEngine::IScene {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DebugScene() = default;
	~DebugScene() = default;

	void Init() override;

	void Update() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unique_ptr<SakuEngine::BaseCamera> camera3D_;
	std::unique_ptr<SakuEngine::BasePunctualLight> light_;

	//--------- functions ----------------------------------------------------

};
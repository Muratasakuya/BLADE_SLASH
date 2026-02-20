#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>

//============================================================================
//	FollowCameraOrbitTranslationComposer class
//	カメラの最終的な座標を構成するクラス
//============================================================================
class FollowCameraOrbitTranslationComposer :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraOrbitTranslationComposer() = default;
	~FollowCameraOrbitTranslationComposer() = default;

	// 初期化
	void Init() override {}

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override {}

	//--------- accessor -----------------------------------------------------

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::OrbitTranslationComposer;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
};
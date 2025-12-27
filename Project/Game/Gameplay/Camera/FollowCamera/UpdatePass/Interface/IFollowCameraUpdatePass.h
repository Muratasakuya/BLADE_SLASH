#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/Direction.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Service/FollowCameraFrameService.h>

// imgui
#include <imgui.h>

//============================================================================
//	IFollowCameraUpdatePass class
//	カメラの更新を行うパスのインターフェース
//============================================================================
class IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IFollowCameraUpdatePass() = default;
	virtual ~IFollowCameraUpdatePass() = default;

	// 初期化
	virtual void Init() = 0;

	// 更新
	virtual void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) = 0;

	// 処理開始
	virtual void Begin([[maybe_unused]] FollowCameraContext& context) {}

	// エディター
	virtual void ImGui() {}

	//--------- accessor -----------------------------------------------------

	// 処理依存オブジェクトを設定
	void BindDependencies(const FollowCameraDependencies& dependencies) { dependencies_ = dependencies; }

	// 識別IDの取得
	virtual FollowCameraUpdatePassID GetID() const = 0;
protected:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 処理対象カメラ
	FollowCameraDependencies dependencies_{};
};
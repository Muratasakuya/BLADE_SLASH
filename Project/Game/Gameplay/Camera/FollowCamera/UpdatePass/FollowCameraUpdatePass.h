#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/EditorModuleRegistry.h>
#include <Game/Scenes/Game/GameState/GameSceneState.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Service/FollowCameraRuntimeDebug.h>

// c++
#include <vector>
#include <memory>

//============================================================================
//	FollowCameraUpdatePass class
//	追従カメラのパス更新を行うクラス
//============================================================================
class FollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraUpdatePass() = default;
	~FollowCameraUpdatePass() = default;

	// 初期化
	void Init();

	// 更新
	void Update(FollowCamera& followCamera, GameSceneState sceneState);

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	// 処理依存オブジェクトを設定
	void BindDependencies(const FollowCameraDependencies& dependencies);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 登録されているクラスを管理するレジストリ
	using PassRegistry = SakuEngine::EditorModuleRegistry<IFollowCameraUpdatePass, FollowCameraUpdatePassID>;

	// 更新パス
	std::vector<std::unique_ptr<IFollowCameraUpdatePass>> updatePasses_;

	// 処理コンテキスト
	FollowCameraContext context_;
	// 提供されているサービス
	FollowCameraFrameService frameService_;

	// エディター
	FollowCameraUpdatePassID editPassID_ = FollowCameraUpdatePassID::LookInputSmoother;

	//--------- functions ----------------------------------------------------

	// サービスの構築
	void BuildFrameService();

	// 一致するIDからパスを取得
	IFollowCameraUpdatePass* GetPassByID(FollowCameraUpdatePassID id);
};
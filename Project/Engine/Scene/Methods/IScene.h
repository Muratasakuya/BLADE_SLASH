#pragma once

//============================================================================
//	include
//============================================================================

namespace SakuEngine {

	// front
	class SceneView;
	class SceneManager;

	//============================================================================
	//	Scene
	//============================================================================

	enum class Scene {

		Debug,
		Title,
		Game,
	};

	//============================================================================
	//	IScene class
	//	シーンの基底クラス
	//============================================================================
	class IScene {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IScene() = default;
		virtual ~IScene() = default;

		virtual void Init() = 0;

		virtual void Update() = 0;

		virtual void BeginFrame() {}
		virtual void EndFrame() {}

		//--------- accessor -----------------------------------------------------

		void SetPtr(SceneView* sceneView, SceneManager* sceneManager);

		bool IsFinishGame() const { return isFinishGame_; }
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		SceneView* sceneView_;
		SceneManager* sceneManager_;

		bool isFinishGame_ = false;;
	};
}; // SakuEngine
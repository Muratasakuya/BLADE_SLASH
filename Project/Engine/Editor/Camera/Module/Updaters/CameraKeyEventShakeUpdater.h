#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/Module/Interface/ICameraKeyEventUpdater.h>

namespace SakuEngine {

	//============================================================================
	//	CameraKeyEventShakeUpdater class
	//	画面シェイク処理を行うカメラキーイベント更新クラス
	//============================================================================
	class CameraKeyEventShakeUpdater :
		public ICameraKeyEventUpdater {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		CameraKeyEventShakeUpdater() = default;
		~CameraKeyEventShakeUpdater() = default;

		// 初期化
		void Init();

		// 更新
		void Update();

		// 処理開始
		void Start();
		// 処理終了
		void End();

		// エディター
		void ImGui();

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data) const;

		//--------- accessor -----------------------------------------------------

		CameraKeyEventUpdaterID GetID() const override { return CameraKeyEventUpdaterID::ScreenShake; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------



		//--------- functions ----------------------------------------------------

	};
}
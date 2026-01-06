#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/Module/Interface/ICameraKeyEventUpdater.h>
#include <Engine/Utility/Timer/StateTimer.h>

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
		void Init() override;

		// 更新
		void Update() override;

		// 処理開始
		void Start() override;
		// 処理終了
		void End() override;

		// エディター
		void ImGui() override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- accessor -----------------------------------------------------

		static constexpr CameraKeyEventUpdaterID ID = CameraKeyEventUpdaterID::ScreenShake;
		CameraKeyEventUpdaterID GetID() const override { return CameraKeyEventUpdaterID::ScreenShake; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		Vector3 intensity_; // シェイク強度
		StateTimer timer_;  // 時間
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/Module/Interface/ICameraKeyEventUpdater.h>
#include <Engine/Utility/Timer/StateTimer.h>

namespace SakuEngine {

	//============================================================================
	//	CameraKeyEventWaitUpdater class
	//	キーの位置で停止させるカメラキーイベント更新クラス
	//============================================================================
	class CameraKeyEventWaitUpdater :
		public ICameraKeyEventUpdater {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		CameraKeyEventWaitUpdater() = default;
		~CameraKeyEventWaitUpdater() = default;

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

		CameraKeyEventUpdaterID GetID() const override { return CameraKeyEventUpdaterID::Stop; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		StateTimer timer_; // 時間
	};
} // SakuEngine
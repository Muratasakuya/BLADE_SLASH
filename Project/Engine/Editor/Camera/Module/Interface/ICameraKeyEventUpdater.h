#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Editor/Camera/Module/CameraKeyEventUpdaterID.h>

namespace SakuEngine {

	//============================================================================
	//	ICameraKeyEventUpdater class
	//	カメラエディターのキーイベント更新インターフェース
	//============================================================================
	class ICameraKeyEventUpdater {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		ICameraKeyEventUpdater() = default;
		virtual ~ICameraKeyEventUpdater() = default;

		// 初期化
		virtual void Init() = 0;

		// 更新
		virtual void Update() = 0;

		// 処理開始
		virtual void Start() = 0;
		// 処理終了
		virtual void End() = 0;

		// エディター
		virtual void ImGui() = 0;

		// json
		virtual void FromJson(const Json& data) = 0;
		virtual void ToJson(Json& data) = 0;

		//--------- accessor -----------------------------------------------------

		// 処理対象のカメラ設定
		void SetCamera(BaseCamera* camera) { camera_ = camera; }

		// 識別IDの取得
		virtual CameraKeyEventUpdaterID GetID() const = 0;

		// 処理が終了したか
		bool IsEnd() const { return isEnd_; }
		// イベント処理中、キーを進めるか
		bool IsAdvanceKey() const { return isAdvanceKey_; }
	protected:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 処理対象のカメラ
		BaseCamera* camera_ = nullptr;

		// 処理を終了したか
		bool isEnd_ = false;

		// イベント処理中、キーを進めるか
		// trueならそのまま補間を行い、falseなら現在の位置で停止する
		bool isAdvanceKey_ = true;
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>

//============================================================================
//	FollowCameraZoomOffsetResolver class
//	カメラのピッチ角度とズーム量に応じたオフセットを計算するクラス
//============================================================================
class FollowCameraZoomOffsetResolver :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraZoomOffsetResolver() = default;
	~FollowCameraZoomOffsetResolver() = default;

	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// オフセット距離の取得
	const SakuEngine::Vector3& GetDesiredOffset() const { return desiredOffset_; }

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::ZoomOffsetResolver;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 回転パラメーター
	struct RotateParam {

		float rotateClampX;   // 範囲制限
		float offsetZNear;    // 制限値までの距離
		float clampThreshold; // 補間閾値

		// 距離閾値に応じた補間
		void CalTargetOffsetZ(float distance, float baseTargetZ, float& outTargetZ) const;

		// エディター
		void ImGui(const char* label);

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);
	};

	//--------- variables ----------------------------------------------------

	RotateParam rotatePlusParam_;  // +制限
	RotateParam rotateMinusParam_; // -制限

	// デフォルトのオフセット距離
	SakuEngine::Vector3 defaultOffset_;
	// 目標オフセット距離
	SakuEngine::Vector3 desiredOffset_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};
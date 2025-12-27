#include "CameraKeyEventShakeUpdater.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	CameraKeyEventShakeUpdater classMethods
//============================================================================

void CameraKeyEventShakeUpdater::Init() {

	// 初期化
	isEnd_ = false;
	intensity_ = Vector3::AnyInit(0.8f);
	timer_.target_ = 0.8f;
}

void CameraKeyEventShakeUpdater::Start() {

	// 開始処理
	isEnd_ = false;
	timer_.Reset();
}

void CameraKeyEventShakeUpdater::End() {

	// 終了処理
	isEnd_ = false;
	timer_.Reset();
}

void CameraKeyEventShakeUpdater::Update() {

	// 時間を更新
	timer_.Update();

	// 時間経過で終了
	if (timer_.IsReached()) {

		isEnd_ = true;
		return;
	}

	// シェイク処理
	const SakuEngine::Transform3D& transform = camera_->GetTransform();

	// 時間経過で減衰させる
	float easedT = timer_.easedT_;
	// シェイク強度を減衰
	Vector3 intencity = Vector3::Lerp(intensity_, Vector3::AnyInit(0.0f), easedT);

	// 各軸のランダムオフセットを計算
	Vector3 offset = RandomGenerator::Generate(-1.0f, 1.0f) * intencity;

	// カメラの前方向、右方向を取得
	SakuEngine::Vector3 forward = transform.GetForward();
	SakuEngine::Vector3 right = transform.GetRight();

	// 座標にオフセットを適用
	SakuEngine::Vector3 translation = transform.translation + (forward * offset.z) + (right * offset.x);
	translation.y += offset.y;

	// 座標を設定
	camera_->SetTranslation(translation);
}

void CameraKeyEventShakeUpdater::ImGui() {

	ImGui::Text(std::format("isEnd: {}", isEnd_).c_str());
	ImGui::Checkbox("isAdvanceKey", &isAdvanceKey_);

	ImGui::Separator();

	ImGui::DragFloat3("intensity", &intensity_.x, 0.01f);
	timer_.ImGui("Timer");
}

void CameraKeyEventShakeUpdater::FromJson(const Json& data) {

	isAdvanceKey_ = data.value("isAdvanceKey_", true);
	intensity_ = Vector3::FromJson(data.value("intensity_", Json()));
	timer_.FromJson(data.value("timer_", Json()));
}

void CameraKeyEventShakeUpdater::ToJson(Json& data) {

	data["isAdvanceKey_"] = isAdvanceKey_;
	data["intensity_"] = intensity_.ToJson();
	timer_.ToJson(data["timer_"]);
}
#include "CameraKeyEventWaitUpdater.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

//============================================================================
//	CameraKeyEventWaitUpdater classMethods
//============================================================================

void CameraKeyEventWaitUpdater::Init() {

	// 初期化
	isAdvanceKey_ = false;
	isEnd_ = false;
	timer_.target_ = 1.0f;
}

void CameraKeyEventWaitUpdater::Start() {

	// 開始処理
	isEnd_ = false;
	timer_.Reset();
}

void CameraKeyEventWaitUpdater::End() {

	// 開始処理
	isEnd_ = false;
	timer_.Reset();
}

void CameraKeyEventWaitUpdater::Update() {

	// 時間を更新
	timer_.Update();

	// 時間経過で終了
	if (timer_.IsReached()) {

		isEnd_ = true;
		return;
	}
}

void CameraKeyEventWaitUpdater::ImGui() {

	ImGui::Text(std::format("isEnd: {}", isEnd_).c_str());
	ImGui::Checkbox("isAdvanceKey", &isAdvanceKey_);

	ImGui::Separator();

	timer_.ImGui("Timer");
}

void CameraKeyEventWaitUpdater::FromJson(const Json& data) {

	isAdvanceKey_ = data.value("isAdvanceKey_", true);
	timer_.FromJson(data.value("timer_", Json()));
}

void CameraKeyEventWaitUpdater::ToJson(Json& data) {

	data["isAdvanceKey_"] = isAdvanceKey_;
	timer_.ToJson(data["timer_"]);
}
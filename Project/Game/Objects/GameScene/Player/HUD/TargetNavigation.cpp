#include "TargetNavigation.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Camera/Follow/FollowCamera.h>

//============================================================================
//	TargetNavigation classMethods
//============================================================================

void TargetNavigation::Init() {

	// オブジェクト配列初期化
	objectArray_ = std::make_unique<GameObject2DArray>();
	objectArray_->Init();

	// オブジェクト追加
	objectArray_->Add("navigationFrame", "navigationFrame", "PlayerNavigationUI");
	objectArray_->Add("navigation", "navigationFrame", "PlayerNavigationUI");

	// json適応
	ApplyJson();
}

void TargetNavigation::Update() {

	Vector3 pivotPos = pivotPos_;
	pivotPos.y = pivotOffsetY_;

	// 視点と注視点のビュー変換座標取得
	Matrix4x4 view = camera_->GetViewMatrix();
	Vector3 pivotView = Vector3::Transform(pivotPos, view);
	Vector3 targetView = Vector3::Transform(targetPos_, view);
	// ビュー空間での差分
	Vector3 diffView = targetView - pivotView;

	// 向き
	Vector2 direction = Vector2(diffView.x, -diffView.z).Normalize();

	// スクリーン座標に変換
	screenPivotPos_ = Math::ProjectToScreen(pivotPos, *camera_);
	screenTargetPos_ = Math::ProjectToScreen(targetPos_, *camera_);

	// 現在の角度を計算
	currentAngleRadian_ = std::atan2(direction.y, direction.x);
	// 90度回転させてオフセット
	currentAngleRadian_ += pi * 0.5f;

	// 矢印の表示距離(0.0f ~ piのなかで0.0fで最大、piで最小になるようにする)
	float distance = std::lerp(minDistanceToTarget_, maxDistanceToTarget_,
		EasedValue(distanceEasingType_, 1.0f - std::fabs(Math::WrapPi(currentAngleRadian_)) / pi));

	// 画面内に注視点がある場合は矢印の位置を制限する
	Vector2 screenDiff = screenTargetPos_ - screenPivotPos_;
	float screenLen = Vector2::Length(screenDiff);
	if (screenLen > 1.0f && distance > screenLen) {
		distance = screenLen;
	}

	// 位置
	Vector2 pos = screenPivotPos_ + direction * distance;

	// 座標と回転を設定
	objectArray_->SetTranslation(pos);
	objectArray_->SetRotation(currentAngleRadian_);

	// 更新
	objectArray_->Update();
}

void TargetNavigation::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	if (ImGui::BeginTabBar("ConsoleTabBar")) {

		if (ImGui::BeginTabItem("Param")) {

			ImGui::Text("targetPos: (%.2f, %.2f, %.2f)", targetPos_.x, targetPos_.y, targetPos_.z);
			ImGui::Text("pivotPos:  (%.2f, %.2f, %.2f)", pivotPos_.x, pivotPos_.y, pivotPos_.z);
			ImGui::Text("currentAngleRadian: %.2f", currentAngleRadian_);

			ImGui::Separator();

			ImGui::DragFloat("pivotOffsetY", &pivotOffsetY_, 0.01f);
			ImGui::DragFloat("maxDistanceToTarget", &maxDistanceToTarget_, 0.1f);
			ImGui::DragFloat("minDistanceToTarget", &minDistanceToTarget_, 0.1f);
			Easing::SelectEasingType(distanceEasingType_);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Object")) {

			objectArray_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void TargetNavigation::IsDisplay(bool isDisplay) {

	// 表示するかどうか
	objectArray_->SetAlpha(isDisplay ? 1.0f : 0.0f);
}

void TargetNavigation::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Player/targetNavigationParameter.json", data)) {
		return;
	}

	objectArray_->FromJson(data["ObjectArray"]);

	maxDistanceToTarget_ = data.value("maxDistanceToTarget_", 5.0f);
	minDistanceToTarget_ = data.value("minDistanceToTarget_", 1.0f);
	pivotOffsetY_ = data.value("pivotOffsetY_", 0.0f);
	distanceEasingType_ = EnumAdapter<EasingType>::FromString(data.value("distanceEasingType_", "Linear")).value();
}

void TargetNavigation::SaveJson() {

	Json data;

	objectArray_->ToJson(data["ObjectArray"]);
	data["maxDistanceToTarget_"] = maxDistanceToTarget_;
	data["minDistanceToTarget_"] = minDistanceToTarget_;
	data["pivotOffsetY_"] = pivotOffsetY_;
	data["distanceEasingType_"] = EnumAdapter<EasingType>::ToString(distanceEasingType_);

	JsonAdapter::Save("Player/targetNavigationParameter.json", data);
}
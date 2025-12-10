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

	// 初期化
	isDisable_ = false;
	isDisplay_ = true;
	inFrustumCheck_ = false;
	isBlink_ = false;

	// json適応
	ApplyJson();
}

void TargetNavigation::SetIsDisplay(bool isDisplay) {

	if (isDisplay_ != isDisplay) {
		// 状態が変わったときのみリセット
		alphaTimer_.Reset();
	}

	isDisplay_ = isDisplay;
}

void TargetNavigation::CheckInCamera() {

	if (!inFrustumCheck_) {
		SetIsDisplay(true);
		return;
	}

	// ワールド座標のtargetPosをビュー射影行列でNDC(-1～1)に変換
	Vector3 ndc = Vector3::Transform(targetPos_, camera_->GetViewProjectionMatrix());

	// カメラ範囲内かチェック
	bool inFrustum =
		ndc.x >= -1.0f && ndc.x <= 1.0f &&
		ndc.y >= -1.0f && ndc.y <= 1.0f &&
		ndc.z >= 0.0f && ndc.z <= 1.0f;

	// 範囲外なら矢印を表示、範囲内なら非表示
	SetIsDisplay(!inFrustum);
}

void TargetNavigation::UpdateBlink(float alpha) {

	// 何もさせない
	if (isDisable_) {

		objectArray_->SetAlpha(0.0f);
		return;
	}

	// falseの時は処理しない
	if (!isBlink_) {
		blinkColorTimer_.Reset();
		objectArray_->SetColor(Color::White(alpha));
		return;
	}

	// 時間更新
	blinkColorTimer_.Update();
	// 時間経過でリセット
	if (blinkColorTimer_.IsReached()) {

		blinkColorTimer_.Reset();
	}

	// 色更新
	Color color = Color::Lerp(Color::White(), targetBlinkColor_, blinkColorTimer_.easedT_);
	color.a = alpha;
	objectArray_->SetColor(color);
}

void TargetNavigation::Update() {

	// カメラ範囲内かチェック
	CheckInCamera();

	// αフェードの更新
	alphaTimer_.Update(std::nullopt, false);
	float t = alphaTimer_.t_;
	float alpha = isDisplay_ ? t : 1.0f - t;
	objectArray_->SetAlpha(alpha);

	// 点滅処理
	UpdateBlink(alpha);

	// 時間の更新
	distanceScaleTimer_.Update(std::nullopt, false);
	// 目標時間に達したらリセットしてループ
	if (distanceScaleTimer_.IsReached()) {
		distanceScaleTimer_.Reset();
	}

	// ピボットのYオフセット適応
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

	// 距離スケールの位相
	float phase = distanceScaleTimer_.t_ * pi * 2.0f;
	float distanceScale = minDistanceScale_ + (1.0f - minDistanceScale_) * waveCenter_ + waveAmplitude_ * std::cos(phase);

	// 位置
	Vector2 pos = screenPivotPos_ + direction * distance * distanceScale;

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

			if (ImGui::Button("Disable")) {

				SetIsDisplay(false);
			}
			ImGui::SameLine();
			if (ImGui::Button("Vaild")) {

				SetIsDisplay(true);
			}

			// ワールド座標のtargetPosをビュー射影行列でNDC(-1～1)に変換
			Vector3 ndc = Vector3::Transform(targetPos_, camera_->GetViewProjectionMatrix());

			// カメラ範囲内かチェック
			bool inFrustum =
				ndc.x >= -1.0f && ndc.x <= 1.0f &&
				ndc.y >= -1.0f && ndc.y <= 1.0f &&
				ndc.z >= 0.0f && ndc.z <= 1.0f;

			ImGui::Text(std::format("inFrustum: {}", inFrustum).c_str());

			ImGui::Text("targetPos: (%.2f, %.2f, %.2f)", targetPos_.x, targetPos_.y, targetPos_.z);
			ImGui::Text("pivotPos:  (%.2f, %.2f, %.2f)", pivotPos_.x, pivotPos_.y, pivotPos_.z);
			ImGui::Text("currentAngleRadian: %.2f", currentAngleRadian_);
			ImGui::Checkbox("inFrustumCheck", &inFrustumCheck_);
			ImGui::Checkbox("isBlink", &isBlink_);

			ImGui::Separator();

			ImGui::DragFloat("pivotOffsetY", &pivotOffsetY_, 0.01f);
			ImGui::DragFloat("maxDistanceToTarget", &maxDistanceToTarget_, 0.1f);
			ImGui::DragFloat("minDistanceToTarget", &minDistanceToTarget_, 0.1f);
			ImGui::DragFloat("minDistanceScale", &minDistanceScale_, 0.01f);
			ImGui::DragFloat("waveCenter", &waveCenter_, 0.01f);
			ImGui::DragFloat("waveAmplitude", &waveAmplitude_, 0.01f);
			ImGui::ColorEdit4("targetBlinkColor", &targetBlinkColor_.r);
			Easing::SelectEasingType(distanceEasingType_);

			distanceScaleTimer_.ImGui("distanceScaleTimer");

			alphaTimer_.ImGui("alphaTimer");

			blinkColorTimer_.ImGui("blinkColorTimer");

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Object")) {

			objectArray_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
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
	minDistanceScale_ = data.value("minDistanceScale_", 0.8f);
	waveAmplitude_ = data.value("waveAmplitude_", 0.1f);
	waveCenter_ = data.value("waveCenter_", 0.9f);
	distanceScaleTimer_.FromJson(data.value("distanceScaleTimer_", Json()));
	alphaTimer_.FromJson(data.value("alphaTimer_", Json()));
	blinkColorTimer_.FromJson(data.value("blinkColorTimer_", Json()));
	targetBlinkColor_ = Color::FromJson(data.value("targetBlinkColor_", Json()));
	distanceEasingType_ = EnumAdapter<EasingType>::FromString(data.value("distanceEasingType_", "Linear")).value();
}

void TargetNavigation::SaveJson() {

	Json data;

	objectArray_->ToJson(data["ObjectArray"]);
	data["maxDistanceToTarget_"] = maxDistanceToTarget_;
	data["minDistanceToTarget_"] = minDistanceToTarget_;
	data["pivotOffsetY_"] = pivotOffsetY_;
	data["minDistanceScale_"] = minDistanceScale_;
	data["waveAmplitude_"] = waveAmplitude_;
	data["waveCenter_"] = waveCenter_;
	distanceScaleTimer_.ToJson(data["distanceScaleTimer_"]);
	alphaTimer_.ToJson(data["alphaTimer_"]);
	blinkColorTimer_.ToJson(data["blinkColorTimer_"]);
	data["targetBlinkColor_"] = targetBlinkColor_.ToJson();
	data["distanceEasingType_"] = EnumAdapter<EasingType>::ToString(distanceEasingType_);

	JsonAdapter::Save("Player/targetNavigationParameter.json", data);
}
#include "GameLight.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	GameLight classMethods
//============================================================================

void GameLight::DerivedInit() {

	// 初期化
	currentState_ = State::None;

	// json適用
	ApplyJson();
}

void GameLight::Start() {

	// 状態別に開始
	if (currentState_ == State::None) {

		beginUpdateTimer_.Reset();
		currentState_ = State::BeginUpdating;
	} else if (currentState_ == State::Wait) {

		endUpdateTimer_.Reset();
		currentState_ = State::EndUpdating;
	}
}

void GameLight::Update() {

	switch (currentState_) {
	case GameLight::State::BeginUpdating: {

		// 時間を更新
		beginUpdateTimer_.Update(std::nullopt, false);

		// スポットライト情報を補間
		light_.spot.color = SakuEngine::Color::Lerp(color_.start, color_.end, beginUpdateTimer_.easedT_);
		light_.spot.pos = SakuEngine::Vector3::Lerp(pos_.start, pos_.end, beginUpdateTimer_.easedT_);
		light_.spot.intensity = std::lerp(intensity_.start, intensity_.end, beginUpdateTimer_.easedT_);
		light_.spot.distance = std::lerp(distance_.start, distance_.end, beginUpdateTimer_.easedT_);
		// 補間が終了次第待ち状態にする
		if (beginUpdateTimer_.IsReached()) {

			currentState_ = State::Wait;
		}
		break;
	}
	case GameLight::State::EndUpdating: {

		// 時間を更新
		endUpdateTimer_.Update(std::nullopt, false);

		// 逆補間して元に戻す
		light_.spot.color = SakuEngine::Color::Lerp(color_.end, color_.start, endUpdateTimer_.easedT_);
		light_.spot.pos = SakuEngine::Vector3::Lerp(pos_.end, pos_.start, endUpdateTimer_.easedT_);
		light_.spot.intensity = std::lerp(intensity_.end, intensity_.start, endUpdateTimer_.easedT_);
		light_.spot.distance = std::lerp(distance_.end, distance_.start, endUpdateTimer_.easedT_);

		// 補間が終了次第何もしない状態にする
		if (endUpdateTimer_.IsReached()) {

			currentState_ = State::None;
		}
		break;
	}
	}
}

void GameLight::DerivedImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	if (ImGui::Button("Start")) {

		Start();
	}
	ImGui::Separator();

	ImGui::Text("Color");
	ImGui::ColorEdit4("Start Color", &color_.start.r);
	ImGui::ColorEdit4("End Color", &color_.end.r);

	ImGui::Text("Pos");
	ImGui::DragFloat3("Start Pos", &pos_.start.x, 0.1f);
	ImGui::DragFloat3("End Pos", &pos_.end.x, 0.1f);

	ImGui::Text("Intensity");
	ImGui::DragFloat("Start Intensity", &intensity_.start, 0.01f);
	ImGui::DragFloat("End Intensity", &intensity_.end, 0.01f);

	ImGui::Text("Distance");
	ImGui::DragFloat("Start Distance", &distance_.start, 0.01f);
	ImGui::DragFloat("End Distance", &distance_.end, 0.01f);

	beginUpdateTimer_.ImGui("Begin Update Timer");
	endUpdateTimer_.ImGui("End Update Timer");
}

void GameLight::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("GameLight/lightParameter.json", data)) {
		return;
	}

	color_.start = SakuEngine::Color::FromJson(data.value("startColor", Json()));
	color_.end = SakuEngine::Color::FromJson(data.value("endColor", Json()));
	pos_.start = SakuEngine::Vector3::FromJson(data.value("startPos", Json()));
	pos_.end = SakuEngine::Vector3::FromJson(data.value("endPos", Json()));
	intensity_.start = data.value("startIntensity", 0.0f);
	intensity_.end = data.value("endIntensity", 0.0f);
	distance_.start = data.value("startDistance", 0.0f);
	distance_.end = data.value("endDistance", 0.0f);

	beginUpdateTimer_.FromJson(data.value("beginUpdateTimer", Json()));
	endUpdateTimer_.FromJson(data.value("endUpdateTimer", Json()));

	// ライトの情報を読み込み
	light_.directional.color = SakuEngine::Color::FromJson(data.value("directionalColor", Json()));
	light_.directional.direction = SakuEngine::Vector3::FromJson(data.value("directionalDirection", Json()));
	light_.directional.intensity = data.value("directionalIntensity", 0.0f);

	light_.point.color = SakuEngine::Color::FromJson(data.value("pointColor", Json()));
	light_.point.pos = SakuEngine::Vector3::FromJson(data.value("pointPos", Json()));
	light_.point.intensity = data.value("pointIntensity", 0.0f);
	light_.point.radius = data.value("pointRadius", 0.0f);
	light_.point.decay = data.value("pointDecay", 0.0f);

	light_.spot.direction = SakuEngine::Vector3::FromJson(data.value("spotDirection", Json()));
	light_.spot.decay = data.value("spotDecay", 0.0f);
	light_.spot.cosAngle = data.value("spotCosAngle", 0.0f);
	light_.spot.cosFalloffStart = data.value("spotCosFalloffStart", 0.0f);
}

void GameLight::SaveJson() {

	Json data;

	data["startColor"] = color_.start.ToJson();
	data["endColor"] = color_.end.ToJson();
	data["startPos"] = pos_.start.ToJson();
	data["endPos"] = pos_.end.ToJson();
	data["startIntensity"] = intensity_.start;
	data["endIntensity"] = intensity_.end;
	data["startDistance"] = distance_.start;
	data["endDistance"] = distance_.end;

	beginUpdateTimer_.ToJson(data["beginUpdateTimer"]);
	endUpdateTimer_.ToJson(data["endUpdateTimer"]);

	// ライトの情報を保存
	data["directionalColor"] = light_.directional.color.ToJson();
	data["directionalDirection"] = light_.directional.direction.ToJson();
	data["directionalIntensity"] = light_.directional.intensity;

	data["pointColor"] = light_.point.color.ToJson();
	data["pointPos"] = light_.point.pos.ToJson();
	data["pointIntensity"] = light_.point.intensity;
	data["pointRadius"] = light_.point.radius;
	data["pointDecay"] = light_.point.decay;

	data["spotDirection"] = light_.spot.direction.ToJson();
	data["spotDecay"] = light_.spot.decay;
	data["spotCosAngle"] = light_.spot.cosAngle;
	data["spotCosFalloffStart"] = light_.spot.cosFalloffStart;

	SakuEngine::JsonAdapter::Save("GameLight/lightParameter.json", data);
}
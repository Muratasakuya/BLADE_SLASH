#include "PunctualLight.h"

//============================================================================
//	include
//============================================================================

// imgui
#include <imgui.h>

//============================================================================
//	Light Methods
//============================================================================

void DirectionalLight::Init() {

	color = Color::White();
	direction = { 0.0f,-1.0f,0.0f };
	intensity = 1.4f;
}

void DirectionalLight::ImGui(float itemWidth) {

	ImGui::SeparatorText("DirectionalLight");

	ImGui::PushItemWidth(itemWidth);

	ImGui::ColorEdit4("color##Directional", &color.r);
	if (ImGui::DragFloat3("direction##Directional", &direction.x, 0.01f)) {

		direction = Vector3::Normalize(direction);
	}
	ImGui::DragFloat("intensity##Directional", &intensity, 0.01f);

	ImGui::PopItemWidth();
}

void PointLight::Init() {

	color = Color::White();
	pos = { 0.0f,8.0f,0.0f };
	radius = 5.0f;
	intensity = 0.8f;
	decay = 1.0f;
}

void PointLight::ImGui(float itemWidth) {

	ImGui::SeparatorText("PointLight");

	ImGui::PushItemWidth(itemWidth);

	ImGui::ColorEdit4("color##PointLight", &color.r);
	ImGui::DragFloat3("pos##PointLight", &pos.x, 0.1f);
	ImGui::DragFloat("radius##PointLight", &radius, 0.01f);
	ImGui::DragFloat("intensity##PointLight", &intensity, 0.01f);
	ImGui::DragFloat("decay##PointLight", &decay, 0.01f);

	ImGui::PopItemWidth();
}

void SpotLight::Init() {

	color = Color::White();
	pos = { 0.0f,8.0f,0.0f };
	distance = 10.0f;
	intensity = 0.8f;
	direction = { 0.0f,-1.0f,0.0f };
	decay = 1.0f;
	cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	cosFalloffStart = 1.0f;
}

void SpotLight::ImGui(float itemWidth) {

	ImGui::SeparatorText("SpotLight");

	ImGui::PushItemWidth(itemWidth);

	ImGui::ColorEdit4("color##SpotLight", &color.r);
	ImGui::DragFloat3("pos##SpotLight", &pos.x, 0.1f);
	if (ImGui::DragFloat3("direction##SpotLight", &direction.x, 0.01f)) {

		direction = Vector3::Normalize(direction);
	}
	ImGui::DragFloat("distance##SpotLight", &distance, 0.01f);
	ImGui::DragFloat("intensity##SpotLight", &intensity, 0.01f);
	ImGui::DragFloat("decay##SpotLight", &decay, 0.01f);
	ImGui::DragFloat("cosAngle##SpotLight", &cosAngle, 0.01f);
	ImGui::DragFloat("cosFalloffStart##SpotLight", &cosFalloffStart, 0.01f);

	ImGui::PopItemWidth();
}

//============================================================================
//	BasePunctualLight classMethods
//============================================================================

void BasePunctualLight::Init() {

	// 各lightの初期化
	light_.directional.Init();
	light_.point.Init();
	light_.spot.Init();

	// 継承先の初期化
	DerivedInit();
}

void BasePunctualLight::ImGui() {

	// 各lightのImGui表示
	light_.directional.ImGui(itemWidth_);
	light_.point.ImGui(itemWidth_);
	light_.spot.ImGui(itemWidth_);

	ImGui::Separator();

	// 継承先のImGui
	DerivedImGui();
}
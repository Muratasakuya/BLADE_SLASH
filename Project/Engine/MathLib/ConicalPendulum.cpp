#include "ConicalPendulum.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>

// imgui
#include <imgui.h>

//============================================================================
//	ConicalPendulum classMethods
//============================================================================

void ConicalPendulum::Init() {

	// 初期化値
	anchor = Vector3(0.0f, 16.0f, 0.0f);
	length = 8.0f;
	angularVelocity = 1.0f;
	halfApexAngle = pi / 6.0f;
	angle = 0.0f;
	minAngle = 0.0f;
	maxAngle = pi;
}

void ConicalPendulum::Update() {

	// 角速度を計算
	float angularSpeed = std::sqrt(9.8f / (length * std::cos(halfApexAngle)));
	if (0.0f <= angularVelocity) {

		angularVelocity = angularSpeed;
	} else {

		angularVelocity = -angularSpeed;
	}
	// 角度を更新
	angle += angularVelocity * GameTimer::GetDeltaTime();
	if (angle > maxAngle) {

		// ここで反転
		angle = maxAngle;
		angularVelocity = -angularSpeed;
	} else if (angle < minAngle) {

		// ここで反転
		angle = minAngle;
		angularVelocity = angularSpeed;
	}

	// 半径と高さ
	float radius = std::sin(halfApexAngle) * length;
	float height = std::cos(halfApexAngle) * length;

	// 現在の位置を計算
	currentPos.x = anchor.x + radius * std::cos(angle);
	currentPos.y = anchor.y - height;
	currentPos.z = anchor.z + radius * std::sin(angle);
}

void ConicalPendulum::DrawDebugLine() {

	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	// 支点から現在位置への線を描画
	lineRenderer->DrawLine3D(anchor, currentPos, Color::Cyan());

	// アンカー位置、現在位置を球で描画
	lineRenderer->DrawSphere(6, 0.4f, anchor, Color::Cyan());
	lineRenderer->DrawSphere(6, 0.4f, currentPos, Color::Cyan());

	// 円錐振り子の円の半径と高さ
	float radius = std::sin(halfApexAngle) * length;
	float height = std::cos(halfApexAngle) * length;

	// 円の中心
	Vector3 center(anchor.x, anchor.y - height, anchor.z);

	// 分割数
	const int32_t kDiv = 64;

	// ここで minAngle ～ maxAngle の弧だけ描く
	float startAngle = minAngle;
	float endAngle = maxAngle;
	float step = (endAngle - startAngle) / kDiv;

	// 最初の点
	Vector3 prev(
		center.x + radius * std::cos(startAngle),
		center.y,
		center.z + radius * std::sin(startAngle));

	for (int32_t i = 1; i <= kDiv; ++i) {

		float a = startAngle + step * i;

		Vector3 p(center.x + radius * std::cos(a),
			center.y,
			center.z + radius * std::sin(a));

		lineRenderer->DrawLine3D(prev, p, Color::Yellow());
		prev = p;
	}
}

void ConicalPendulum::ImGui() {

	ImGui::SeparatorText("Runtime");

	ImGui::Checkbox("isDrawDebug", &isDrawDebug);
	ImGui::Checkbox("isEditUpdate", &isEditUpdate);
	ImGui::Text("currentPos: (%.2f, %.2f, %.2f)", currentPos.x, currentPos.y, currentPos.z);
	ImGui::Text("angle: %.2f", angle);

	ImGui::SeparatorText("Edit");

	if (ImGui::Button("Reset")) {
		Init();
	}

	ImGui::DragFloat3("anchor", &anchor.x, 0.1f);
	ImGui::DragFloat("length", &length, 0.01f);
	ImGui::DragFloat("halfApexAngle", &halfApexAngle, 0.01f, 0.01f, pi / 2.0f);
	ImGui::DragFloat("minAngle", &minAngle, 0.01f, -pi * 2.0f, pi * 2.0f);
	ImGui::DragFloat("maxAngle", &maxAngle, 0.01f, -pi * 2.0f, pi * 2.0f);

	if (isEditUpdate) {

		Update();
	}
	if (isDrawDebug) {

		DrawDebugLine();
	}
}

void ConicalPendulum::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	anchor = Vector3::FromJson(data.value("anchor", Json()));
	currentPos = Vector3::FromJson(data.value("currentPos", Json()));
	length = data["length"];
	halfApexAngle = data["halfApexAngle"];
	angle = data["angle"];
	angularVelocity = data["angularVelocity"];
	minAngle = data["minAngle"];
	maxAngle = data["maxAngle"];
}

void ConicalPendulum::ToJson(Json& data) {

	data["anchor"] = anchor.ToJson();
	data["currentPos"] = currentPos.ToJson();
	data["length"] = length;
	data["halfApexAngle"] = halfApexAngle;
	data["angle"] = angle;
	data["angularVelocity"] = angularVelocity;
	data["minAngle"] = minAngle;
	data["maxAngle"] = maxAngle;
}
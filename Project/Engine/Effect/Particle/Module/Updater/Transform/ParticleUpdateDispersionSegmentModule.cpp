#include "ParticleUpdateDispersionSegmentModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateDispersionSegmentModule classMethods
//============================================================================

void ParticleUpdateDispersionSegmentModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		break;
	}
	default:
		break;
	}
}

void ParticleUpdateDispersionSegmentModule::Init() {

	// 初期化値
	isRefSpawnPos_ = true;
	centerPos_ = Vector3(0.0f, 8.0f, 0.0f);
	shape_ = Shape::Segment;
	segmentParams_.origin = Vector3(-4.0f, 0.0f, 0.0f);
	segmentParams_.target = Vector3(4.0f, 0.0f, 0.0f);
	circleParams_.radius = 4.0f;
	rectParams_.size = Vector2::AnyInit(8.0f);

	divisionCount_ = 2u;
	delay_ = 0.2f;
	targetPosRange_ = Vector3::AnyInit(1.0f);
	targetSize = 1.0f;
}

void ParticleUpdateDispersionSegmentModule::Execute(
	CPUParticle::ParticleData& /*particle*/, float /*deltaTime*/) {
}

void ParticleUpdateDispersionSegmentModule::ImGui() {

	//============================================================================
	//	Parameters
	//============================================================================

	ImGui::SeparatorText("Parameters");

	ImGui::Checkbox("isRefSpawnPos", &isRefSpawnPos_);

	int32_t divisionCount = static_cast<int32_t>(divisionCount_);
	ImGui::DragInt("divisionCount", &divisionCount, 1);
	divisionCount_ = static_cast<uint32_t>(divisionCount);

	ImGui::DragFloat3("targetPosRange", &targetPosRange_.x, 0.01f);
	ImGui::DragFloat("targetSize", &targetSize, 0.01f);

	//============================================================================
	//	Shape
	//============================================================================

	ImGui::SeparatorText("Shape");

	EnumAdapter<Shape>::Combo("Shape", &shape_);

	ImGui::DragFloat3("centerPos", &centerPos_.x, 0.01f);

	// 形状ごとのパラメータ表示、線描画
	LineRenderer* lineRenderer = LineRenderer::GetInstance();
	switch (shape_) {
	case ParticleUpdateDispersionSegmentModule::Shape::Segment:

		ImGui::DragFloat3("origin", &segmentParams_.origin.x, 0.01f);
		ImGui::DragFloat3("target", &segmentParams_.target.x, 0.01f);

		// 中心位置からの線分を描画
		lineRenderer->DrawSegment(6, 0.1f, centerPos_ + segmentParams_.origin,
			centerPos_ + segmentParams_.target, Color::Cyan());
		break;
	case ParticleUpdateDispersionSegmentModule::Shape::Circle:

		ImGui::DragFloat("radius", &circleParams_.radius, 0.01f);

		// 中心位置からの円を描画
		lineRenderer->DrawCircle(6, circleParams_.radius, centerPos_, Color::Cyan());
		break;
	case ParticleUpdateDispersionSegmentModule::Shape::Rect:

		ImGui::DragFloat2("size", &rectParams_.size.x, 0.01f);

		// 中心位置からの矩形を描画
		lineRenderer->DrawRect(rectParams_.size, centerPos_, Color::Cyan());
		break;
	}
}

Json ParticleUpdateDispersionSegmentModule::ToJson() {

	Init();

	Json data;

	data["shape_"] = EnumAdapter<Shape>::ToString(shape_);
	centerPos_ = Vector3::FromJson(data.value("centerPos_", Json()));
	segmentParams_.origin = Vector3::FromJson(data.value("segmentParams_.origin", Json()));
	segmentParams_.target = Vector3::FromJson(data.value("segmentParams_.target", Json()));
	circleParams_.radius = data.value("circleParams_.radius", Json());
	rectParams_.size = Vector2::FromJson(data.value("rectParams_.size", Json()));

	isRefSpawnPos_ = data.value("isRefSpawnPos_", true);
	divisionCount_ = data.value("divisionCount_", 1u);
	delay_ = data.value("delay_", 0.2f);
	targetPosRange_ = Vector3::FromJson(data.value("targetPosRange_", Json()));
	targetSize = data.value("targetSize", 1.0f);

	return data;
}

void ParticleUpdateDispersionSegmentModule::FromJson(const Json& data) {

	shape_ = EnumAdapter<Shape>::FromString(data.value("shape_", "Segment")).value();
	centerPos_ = Vector3::FromJson(data.value("centerPos_", Json()));
	segmentParams_.origin = Vector3::FromJson(data.value("segmentParams_.origin", Json()));
	segmentParams_.target = Vector3::FromJson(data.value("segmentParams_.target", Json()));
	circleParams_.radius = data.value("circleParams_.radius", Json());
	rectParams_.size = Vector2::FromJson(data.value("rectParams_.size", Json()));

	isRefSpawnPos_ = data.value("isRefSpawnPos_", true);
	divisionCount_ = data.value("divisionCount_", 1u);
	delay_ = data.value("delay_", 0.2f);
	targetPosRange_ = Vector3::FromJson(data.value("targetPosRange_", Json()));
	targetSize = data.value("targetSize", 1.0f);
}
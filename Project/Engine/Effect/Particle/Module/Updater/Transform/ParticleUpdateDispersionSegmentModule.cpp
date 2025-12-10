#include "ParticleUpdateDispersionSegmentModule.h"

using namespace SakuEngine;

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
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// 参照する中心位置
	const Vector3 baseCenter = isRefSpawnPos_ ? particle.spawnTranlation : centerPos_;

	// 外側への層：phaseIndex を利用して 0..divisionCount_-1 で循環
	const uint32_t div = (divisionCount_ == 0u) ? 1u : divisionCount_;
	const uint32_t layerIndex = (div == 0u) ? 0u : (particle.phaseIndex % div); // :contentReference[oaicite:9]{index=9}
	const float layerT = (static_cast<float>(layerIndex) + 1.0f) / (static_cast<float>(div) + 1.0f); // 0→外側 比率

	// “角度”は等配列にしても良いが総数が不明なためゴールデンアングルで分散
	const float golden = 2.39996322972865332f; // 137.5deg
	float angle = (particle.hasKeyPathSpawnAngle)
		? particle.keyPathSpawnAngle
		: std::fmod(static_cast<float>(particle.phaseIndex) * golden, 6.283185307179586f);
	if (!particle.hasKeyPathSpawnAngle) {
		particle.keyPathSpawnAngle = angle;
		particle.hasKeyPathSpawnAngle = true; // 初回だけ保存
	}

	// ====== 形状ごとの「中心から外側へ」基準座標を算出 ======================
	auto computeBasePos = [&](Shape s) -> Vector3 {
		switch (s) {
		case Shape::Segment: {
			// セグメントのワールド端点
			const Vector3 A = baseCenter + segmentParams_.origin;
			const Vector3 B = baseCenter + segmentParams_.target;
			const Vector3 mid = (A + B) * 0.5f;
			const Vector3 half = (B - A) * 0.5f;
			const float halfLen = Vector3::Length(half);
			const Vector3 dir = Vector3::Normalize(half);

			// 中心から +/− 交互に外側へ。符号は層内 “周回”で交互に切替
			const int side = ((particle.phaseIndex / div) % 2 == 0) ? +1 : -1;
			const float dist = layerT * halfLen;
			return mid + dir * (static_cast<float>(side) * dist);
		}
		case Shape::Circle: {
			const float r = circleParams_.radius * layerT;
			const Vector3 dir(std::cos(angle), 0.0f, std::sin(angle));
			return baseCenter + dir * r;
		}
		case Shape::Rect: {
			// 角度方向に矩形内で取り得る最大距離 tMax を求め、layerT 倍で内側に
			const Vector3 dir(std::cos(angle), 0.0f, std::sin(angle));
			const float hx = rectParams_.size.x * 0.5f;
			const float hz = rectParams_.size.y * 0.5f;
			const float tx = (std::abs(dir.x) < 1e-6f) ? FLT_MAX : hx / std::abs(dir.x);
			const float tz = (std::abs(dir.z) < 1e-6f) ? FLT_MAX : hz / std::abs(dir.z);
			const float tMax = std::min(tx, tz);
			return baseCenter + dir * (tMax * layerT);
		}
		}
		return baseCenter;
		};

	const Vector3 spawnPos = computeBasePos(shape_); // 割り当てた基準座標

	// ====== 目標位置の算出（基準から targetSize 分だけ真下へ + ランダム） ====
	const Vector3 drop = Vector3(0.0f, -targetSize, 0.0f);
	const Vector3 rand = RandomGenerator::Generate(-targetPosRange_, targetPosRange_); // :contentReference[oaicite:10]{index=10}
	const Vector3 targetPos = spawnPos + drop + rand;

	// ====== 目標方向を向く回転（Yaw のみ） ==================================
	const Vector3 toTarget = Vector3::Normalize(targetPos - spawnPos);
	const float yaw = Math::GetYawRadian(toTarget); // :contentReference[oaicite:11]{index=11}

	particle.transform.rotationMatrix = Matrix4x4::MakeYawMatrix(yaw);

	// ====== 位置・スケールの設定 =============================================
	// 位置は “外側へ割り当てた座標” をベースに固定（※動かしたいなら Lerp でも良い）
	particle.transform.translation = spawnPos;

	// scale: progress(0→1) で 0→targetSize に補間
	const float scale = std::clamp(std::lerp(0.0f, targetSize, particle.progress), 0.0f, targetSize);
	particle.transform.scale = Vector3(particle.transform.scale.x,
		scale, particle.transform.scale.z);

	// ====== 遅延（lifeTime に加算）：初回のみ ================================
	// 「lifeTime にその分 +」という仕様に合わせ、初回だけ加算
	if (!particle.hasKeyPathStart) { // 初期化フラグを流用
		particle.lifeTime += delay_ * static_cast<float>(layerIndex); // :contentReference[oaicite:13]{index=13}
		particle.hasKeyPathStart = true;
	}
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

	ImGui::DragFloat("delay", &delay_, 0.01f);
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

void ParticleUpdateDispersionSegmentModule::FromJson(const Json& data) {

	Init();

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

Json ParticleUpdateDispersionSegmentModule::ToJson() {

	Json data;

	data["shape_"] = EnumAdapter<Shape>::ToString(shape_);
	data["centerPos_"] = centerPos_.ToJson();
	data["segmentParams_.origin"] = segmentParams_.origin.ToJson();
	data["segmentParams_.target"] = segmentParams_.target.ToJson();
	data["circleParams_.radius"] = circleParams_.radius;
	data["rectParams_.size"] = rectParams_.size.ToJson();

	data["isRefSpawnPos_"] = isRefSpawnPos_;
	data["divisionCount_"] = divisionCount_;
	data["delay_"] = delay_;
	data["targetPosRange_"] = targetPosRange_.ToJson();
	data["targetSize"] = targetSize;

	return data;
}

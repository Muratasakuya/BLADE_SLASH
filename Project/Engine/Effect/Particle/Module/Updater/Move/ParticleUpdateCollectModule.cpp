#include "ParticleUpdateCollectModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateCollectModule classMethods
//============================================================================

// Perlin 補間
static inline float FadeCollect(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
// ハッシュ
static inline uint32_t Hash3dCollect(int x, int y, int z, uint32_t s) {
	uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + (uint32_t)z * 362437u + s * 521417u;
	h ^= h >> 13; h *= 1274126177u; h ^= h >> 16;
	return h;
}
static inline float Rnd01Collect(uint32_t h) { return (h & 0x00FFFFFFu) / float(0x01000000); }

void ParticleUpdateCollectModule::Init() {

	// 初期化値
	collectMode_ = CollectMode::Straight;
	isAlignXZToTarget_ = false;
	targetPos_ = Vector3(0.0f, 0.8f, 0.0f);

	// ノイズ初期値
	noiseOctaves_ = 3;
	noiseFrequency_ = 0.5f;
	noiseTimeScale_ = 0.3f;
	noiseAmp_ = Vector3::AnyInit(0.2f);
	noiseFalloffPow_ = 1.0f;
	noiseSeed_ = 1337u;
}

void ParticleUpdateCollectModule::Execute(CPUParticle::ParticleData& particle, float deltaTime) {

	// 目標座標のXZを発生位置に合わせる
	if (isAlignXZToTarget_) {

		targetPos_.x = particle.emitterTranslation.x;
		targetPos_.z = particle.emitterTranslation.z;
	}

	// モード別に更新
	switch (collectMode_) {
	case ParticleUpdateCollectModule::CollectMode::Straight: {

		// 発生位置への補間
		Vector3 lerpPos = Vector3::Lerp(particle.spawnTranlation, targetPos_,
			EasedValue(moveEasing_, particle.progress));

		// 座標を設定
		particle.transform.translation = lerpPos;
		break;
	}
	case ParticleUpdateCollectModule::CollectMode::Random: {

		// 補間値
		float t = EasedValue(moveEasing_, particle.progress);

		// 直線のベース位置
		Vector3 basePos = Vector3::Lerp(particle.spawnTranlation, targetPos_, t);

		// ノイズをサンプルする座標
		Vector3 sample = basePos * noiseFrequency_ + Vector3(0.0f, noiseTimeScale_ * particle.currentTime, 0.0f);

		// FBmノイズからオフセットベクトルを作成
		Vector3 o(FBm(sample, noiseSeed_ + 11u),
			FBm(sample, noiseSeed_ + 23u), FBm(sample, noiseSeed_ + 31u));

		// だんだんtargetに収束させるための減衰 
		float falloff = std::pow(std::max(0.0f, 1.0f - particle.progress), noiseFalloffPow_);
		Vector3 offset(o.x * noiseAmp_.x * falloff, o.y * noiseAmp_.y * falloff, o.z * noiseAmp_.z * falloff);

		// 座標を設定
		particle.transform.translation = basePos + offset;
		break;
	}
	}
}

void ParticleUpdateCollectModule::ImGui() {

	EnumAdapter<CollectMode>::Combo("CollectMode", &collectMode_);
	EnumAdapter<EasingType>::Combo("EasingType", &moveEasing_);
	ImGui::Checkbox("isAlignXZToTarget", &isAlignXZToTarget_);

	ImGui::DragFloat3("targetPos", &targetPos_.x, 0.01f);

	if (collectMode_ == CollectMode::Random) {

		ImGui::DragInt("noiseOctaves", &noiseOctaves_, 1, 1, 8);
		ImGui::DragFloat("noiseFrequency", &noiseFrequency_, 0.01f, 0.01f, 10.0f);
		ImGui::DragFloat("noiseTimeScale", &noiseTimeScale_, 0.01f, 0.0f, 5.0f);
		ImGui::DragFloat3("noiseAmp", &noiseAmp_.x, 0.01f);
		ImGui::DragFloat("noiseFalloffPow", &noiseFalloffPow_, 0.01f, 0.1f, 4.0f);
		ImGui::InputScalar("noiseSeed", ImGuiDataType_U32, &noiseSeed_);
	}
}

Json ParticleUpdateCollectModule::ToJson() {

	Json data;

	data["collectMode_"] = EnumAdapter<CollectMode>::ToString(collectMode_);
	data["moveEasing_"] = EnumAdapter<EasingType>::ToString(moveEasing_);
	data["isAlignXZToTarget_"] = isAlignXZToTarget_;
	data["targetPos_"] = targetPos_.ToJson();

	data["noiseFrequency_"] = noiseFrequency_;
	data["noiseTimeScale_"] = noiseTimeScale_;
	data["noiseAmp_"] = noiseAmp_.ToJson();
	data["noiseFalloffPow_"] = noiseFalloffPow_;
	data["noiseOctaves_"] = noiseOctaves_;
	data["noiseSeed_"] = noiseSeed_;

	return data;
}

void ParticleUpdateCollectModule::FromJson(const Json& data) {

	Init();

	collectMode_ = EnumAdapter<CollectMode>::FromString(data.value("collectMode_", "Straight")).value();
	moveEasing_ = EnumAdapter<EasingType>::FromString(data.value("moveEasing_", "Linear")).value();
	isAlignXZToTarget_ = data.value("isAlignXZToTarget_", false);
	targetPos_.FromJson(data.value("targetPos_", Json()));

	noiseFrequency_ = data.value("noiseFrequency_", 1.0f);
	noiseTimeScale_ = data.value("noiseTimeScale_", 1.0f);
	noiseAmp_.FromJson(data.value("noiseAmp_", Json()));
	noiseFalloffPow_ = data.value("noiseFalloffPow_", 1.0f);
	noiseOctaves_ = data.value("noiseOctaves_", 3);
	noiseSeed_ = data.value("noiseSeed_", 0u);
}

float ParticleUpdateCollectModule::Noise3(const Vector3& p, uint32_t s) const {

	const int X = (int)std::floor(p.x);
	const int Y = (int)std::floor(p.y);
	const int Z = (int)std::floor(p.z);

	const float fx = p.x - X;
	const float fy = p.y - Y;
	const float fz = p.z - Z;

	const float ux = FadeCollect(fx);
	const float uy = FadeCollect(fy);
	const float uz = FadeCollect(fz);

	auto L = [&](int dx, int dy, int dz) {
		return Rnd01Collect(Hash3dCollect(X + dx, Y + dy, Z + dz, s));
		};

	float x00 = std::lerp(L(0, 0, 0), L(1, 0, 0), ux);
	float x10 = std::lerp(L(0, 1, 0), L(1, 1, 0), ux);
	float x01 = std::lerp(L(0, 0, 1), L(1, 0, 1), ux);
	float x11 = std::lerp(L(0, 1, 1), L(1, 1, 1), ux);
	float y0 = std::lerp(x00, x10, uy);
	float y1 = std::lerp(x01, x11, uy);

	// [-1,1]
	return std::lerp(y0, y1, uz) * 2.0f - 1.0f;
}

float ParticleUpdateCollectModule::FBm(const Vector3& p, uint32_t s) const {

	float a = 1.0f;
	float f = 1.0f;
	float sum = 0.0f;
	float norm = 0.0f;

	for (int i = 0; i < noiseOctaves_; i++) {

		sum += a * Noise3(p * f, s + i * 101u);
		norm += a;
		a *= 0.5f;
		f *= 2.0f;
	}
	return sum / (std::max)(1e-6f, norm);
}
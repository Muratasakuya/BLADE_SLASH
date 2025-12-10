#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

//============================================================================
//	ParticleUpdateCollectModule class
//	目標に集まる動きをさせる更新モジュール
//============================================================================
namespace SakuEngine {

class ParticleUpdateCollectModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateCollectModule() = default;
	~ParticleUpdateCollectModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "Collect"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Collect;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 集まり方
	enum class CollectMode {

		Straight, // 直線
		Random,   // ランダム
	};

	//--------- variables ----------------------------------------------------

	// 集まり方
	CollectMode collectMode_;

	// 発生位置のXZ座標を目標座標に設定するか
	bool isAlignXZToTarget_;

	// 目標座標
	Vector3 targetPos_;
	EasingType moveEasing_;

	// ランダム収束用ノイズ設定
	float noiseFrequency_;  // 空間周波数
	float noiseTimeScale_;  // 時間の進み
	Vector3 noiseAmp_;      // 最大振れ幅
	float noiseFalloffPow_; // 収束カーブの指数
	int32_t noiseOctaves_;
	uint32_t noiseSeed_;

	//--------- functions ----------------------------------------------------

	// ノイズ生成関数
	float Noise3(const Vector3& p, uint32_t s) const;
	float FBm(const Vector3& p, uint32_t s) const;
};

}; // SakuEngine

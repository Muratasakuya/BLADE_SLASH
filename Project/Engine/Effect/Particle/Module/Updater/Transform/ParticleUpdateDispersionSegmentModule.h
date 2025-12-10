#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

//============================================================================
//	ParticleUpdateDispersionSegmentModule class
//	外側に広がるようにサイズの線分補間を行うモジュール
//============================================================================
class ParticleUpdateDispersionSegmentModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateDispersionSegmentModule() = default;
	~ParticleUpdateDispersionSegmentModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetCommand(const ParticleCommand& command) override;

	const char* GetName() const override { return "DispersionSegment"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::DispersionSegment;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 発生源の形状
	enum class Shape {

		Segment, // 線の中心から外側へ
		Circle,  // 円の中心から外側へ
		Rect     // 矩形の中心から外側へ
	};

	// 形状ごとのパラメータ
	struct Segment {

		Vector3 origin; // 始点
		Vector3 target; // 終点
	};
	struct Circle {

		float radius; // 半径
	};
	struct Rect {

		Vector2 size; // サイズ
	};

	//--------- variables ----------------------------------------------------

	// 発生地点を参照するか
	bool isRefSpawnPos_;

	Vector3 centerPos_; // 中心位置

	// 形状
	Shape shape_;
	Segment segmentParams_;
	Circle circleParams_;
	Rect rectParams_;

	// 発生パラメータ
	uint32_t divisionCount_; // 遅延ごとに発生させる分割数(4で発生数が12なら遅延ごとに3個ずつどんどん外側にずらして発生させる)
	float delay_;            // 遅延時間(lifeTimeにその分+する)
	Vector3 targetPosRange_; // 発生位置から真下に下ろした位置からのランダム範囲
	float targetSize;        // 補間目標サイズ

	//--------- functions ----------------------------------------------------

};
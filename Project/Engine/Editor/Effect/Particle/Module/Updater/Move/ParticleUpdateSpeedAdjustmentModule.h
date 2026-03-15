#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

namespace SakuEngine {

	//============================================================================
	//	ParticleUpdateSpeedAdjustmentModule class
	//	パーティクルの速度を加速、減衰させる
	//============================================================================
	class ParticleUpdateSpeedAdjustmentModule :
		public ICPUParticleUpdateModule {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		ParticleUpdateSpeedAdjustmentModule() = default;
		~ParticleUpdateSpeedAdjustmentModule() = default;

		void Init() override;

		void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

		void ImGui() override;

		// json
		Json ToJson() override;
		void FromJson(const Json& data) override;

		//--------- accessor -----------------------------------------------------

		const char* GetName() const override { return "SpeedAdjustment"; }

		//-------- registryID ----------------------------------------------------

		static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::SpeedAdjustment;
		ParticleUpdateModuleID GetID() const override { return ID; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// 符号
		enum class Sign {

			Plus,
			Minus
		};

		//--------- variables ----------------------------------------------------

		// 計算符号
		Sign calSign_;

		// 値
		float adjustmentValue_;
	};
} // SakuEngine
#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Effect/Particle/Command/ParticleCommand.h>

namespace SakuEngine {

// front

class ParticleSystem;

//============================================================================
//	EffectCommandRouter class
//	パーティクルシステムにコマンドを送る
//============================================================================
class EffectCommandRouter {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectCommandRouter() = default;
	~EffectCommandRouter() = default;

	// システム全体にコマンドを送る
	static void Send(ParticleSystem* system, const ParticleCommand& command);
};

}; // SakuEngine
